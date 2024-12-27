/******************************************************************************\

                 This file is part of the Buildbotics firmware.

                   Copyright (c) 2015 - 2019, Buildbotics LLC
                              All rights reserved.

      This file ("the software") is free software: you can redistribute it
      and/or modify it under the terms of the GNU General Public License,
       version 2 as published by the Free Software Foundation. You should
       have received a copy of the GNU General Public License, version 2
      along with the software. If not, see <http://www.gnu.org/licenses/>.

      The software is distributed in the hope that it will be useful, but
           WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
                Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public
                 License along with the software.  If not, see
                        <http://www.gnu.org/licenses/>.

                 For information regarding this software email:
                   "Joseph Coffland" <joseph@buildbotics.com>

\******************************************************************************/

#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/irq.h>      // For irqreturn_t
#include <linux/clk.h>      // For clk_get_rate()
#include <linux/poll.h>     // For POLLIN
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/termios.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>

/* Function prototypes */
static void _rx_chars(void);
static void _tx_chars(void);
static int _read_status(void);
static void _write_status(int status);
static struct ktermios *_get_term(void);
static void _set_baud(speed_t baud);
static int _set_term(struct ktermios *term);
static void _flush_input(void);
static void _flush_output(void);
static irqreturn_t _interrupt(int irq, void *id);
static void _enable_tx(void);
static int _tx_enabled(void);
static void _enable_rx(void);
static int _rx_enabled(void);
static int _dev_open(struct inode *inodep, struct file *filep);
static ssize_t _dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset);
static ssize_t _dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);
static int _dev_release(struct inode *inodep, struct file *filep);
static unsigned _dev_poll(struct file *file, poll_table *wait);
static long _dev_ioctl(struct file *file, unsigned cmd, unsigned long arg);
static int _probe(struct platform_device *dev);
static int bbserial_remove(struct platform_device *dev);

/* PL011 registers */
#define UART01x_FR          0x18  /* Flag register */
#define UART01x_FR_TXFF     0x20  /* Transmit FIFO full */
#define UART01x_FR_RXFE     0x10  /* Receive FIFO empty */
#define UART01x_FR_BUSY     0x08  /* UART busy */
#define UART01x_DR          0x00  /* Data register */
#define UART01x_LCRH        0x2C  /* Line Control Register */
#define UART01x_CR          0x30  /* Control Register */
#define UART01x_IMSC        0x38  /* Interrupt Mask Register */
#define UART01x_MIS         0x40  /* Masked Interrupt Status Register */
#define UART01x_ICR         0x44  /* Interrupt Clear Register */

/* PL011 UART Register Bit Definitions */
#define UART011_TXIM       0x20    /* Transmit interrupt mask */
#define UART011_IMSC       0x38    /* Interrupt mask set/clear register */
#define UART011_FR_TXFE    0x80    /* Transmit FIFO empty */
#define UART011_DR_OE      0x800   /* Overrun error */
#define UART011_DR_BE      0x400   /* Break error */
#define UART011_DR_PE      0x200   /* Parity error */
#define UART011_DR_FE      0x100   /* Framing error */
#define UART011_DR_ERR     (UART011_DR_OE|UART011_DR_BE|UART011_DR_PE|UART011_DR_FE)

/* PL011 Line Control Register Bits */
#define UART01x_LCRH_WLEN_5    0x00
#define UART01x_LCRH_WLEN_6    0x20
#define UART01x_LCRH_WLEN_7    0x40
#define UART01x_LCRH_WLEN_8    0x60
#define UART01x_LCRH_FEN       0x10
#define UART01x_LCRH_STP2      0x08
#define UART01x_LCRH_EPS       0x04
#define UART01x_LCRH_PEN       0x02
#define UART01x_LCRH_BRK       0x01

/* PL011 Control Register Bits */
#define UART011_CR             0x30    /* Control Register */
#define UART011_CR_CTSEN       0x8000  /* CTS hardware flow control enable */
#define UART011_CR_RTSEN       0x4000  /* RTS hardware flow control enable */
#define UART011_CR_RTS         0x2000  /* Request to Send */
#define UART011_CR_DTR         0x1000  /* Data Transmit Ready */
#define UART011_CR_RXE         0x0200  /* Receive Enable */
#define UART011_CR_TXE         0x0100  /* Transmit Enable */
#define UART011_CR_LBE         0x0080  /* Loopback Enable */
#define UART011_CR_UARTEN      0x0001  /* UART Enable */

/* PL011 Interrupt Bits */
#define UART011_RXIM           0x10    /* Receive Interrupt Mask */
#define UART011_RTIM           0x40    /* Receive Timeout Interrupt Mask */

/* PL011 Status Bits */
#define UART01x_FR_RI          0x100   /* Ring Indicator */
#define UART01x_FR_CTS         0x20    /* Clear to Send */
#define UART01x_FR_DCD         0x04    /* Data Carrier Detect */
#define UART01x_FR_DSR         0x02    /* Data Set Ready */

/* PL011 Baud Rate Registers */
#define UART011_IBRD          0x24    /* Integer Baud Rate Register */
#define UART011_FBRD          0x28    /* Fractional Baud Rate Register */

/* PL011 Additional Control Bits */
#define UART011_LCRH_SPS      0x80    /* Stick Parity Select */

/* Additional PL011 register definitions */
#define UART011_FR_RI      0x100   /* Ring indicator */
#define UART011_LCRH       0x2C    /* Line control register */
#define UART011_MIS        0x40    /* Masked interrupt status register */
#define UART011_RTIS       0x40    /* Receive timeout interrupt status */
#define UART011_RXIS       0x10    /* Receive interrupt status */
#define UART011_TXIS       0x20    /* Transmit interrupt status */

#define UART011_IFLS      0x34    /* FIFO Level Select Register */
#define UART011_IFLS_RX2_8  (0 << 3)    /* Receive FIFO 2/8 full */
#define UART011_IFLS_TX6_8  (3 << 0)    /* Transmit FIFO 6/8 full */
#define UART01x_CR_UARTEN   0x0001      /* UART Enable */

#define UART011_ICR         0x44    /* Interrupt Clear Register */

static const char __module_license[] __attribute__((section(".modinfo"), used)) = "license=GPL";
MODULE_AUTHOR("Joseph Coffland");
MODULE_DESCRIPTION("BeagleBone Serial Port Driver");
MODULE_VERSION("1.0");

#define DEVICE_NAME "ttyAMA0"
#define BUF_SIZE (1 << 16)

#define UART01x_LCRH_WLEN_bm 0x60

static int debug = 0;
module_param(debug, int, 0660);

struct ring_buf
{
  unsigned char *buf;
  unsigned head;
  unsigned tail;
};

static struct
{
  struct clk *clk;
  struct ring_buf tx_buf;
  struct ring_buf rx_buf;
  spinlock_t lock;
  unsigned open;
  unsigned char __iomem *base;
  wait_queue_head_t read_wait;
  wait_queue_head_t write_wait;
  unsigned irq;
  unsigned im; // interrupt mask

  unsigned brk_errs;
  unsigned parity_errs;
  unsigned frame_errs;
  unsigned overruns;

  int major;
  struct class *class;
  struct device *dev;
  struct ktermios term;
} _port;

#define RING_BUF_INC(BUF, INDEX)                      \
  do                                                  \
  {                                                   \
    (BUF).INDEX = ((BUF).INDEX + 1) & (BUF_SIZE - 1); \
  } while (0)

#define RING_BUF_POP(BUF) RING_BUF_INC(BUF, tail)

#define RING_BUF_PUSH(BUF, C)  \
  do                           \
  {                            \
    (BUF).buf[(BUF).head] = C; \
    mb();                      \
    RING_BUF_INC(BUF, head);   \
  } while (0)

#define RING_BUF_POKE(BUF) (BUF).buf[(BUF).head]
#define RING_BUF_PEEK(BUF) (BUF).buf[(BUF).tail]
#define RING_BUF_SPACE(BUF) ((((BUF).tail) - ((BUF).head + 1)) & (BUF_SIZE - 1))
#define RING_BUF_FILL(BUF) ((((BUF).head) - ((BUF).tail)) & (BUF_SIZE - 1))
#define RING_BUF_CLEAR(BUF)      \
  do                             \
  {                              \
    (BUF).head = (BUF).tail = 0; \
  } while (0)

static unsigned _read(unsigned reg) { return readw_relaxed(_port.base + reg); }

static void _write(unsigned val, unsigned reg)
{
  writew_relaxed(val, _port.base + reg);
}

static void _tx_chars(void)
{
  unsigned fill = RING_BUF_FILL(_port.tx_buf);
  unsigned status;

  while (fill--)
  {
    // Check if UART FIFO full
    status = _read(UART01x_FR);
    if (status & UART01x_FR_TXFF)
      break;

    _write(RING_BUF_PEEK(_port.tx_buf), UART01x_DR);
    mb();
    RING_BUF_POP(_port.tx_buf);
  }

  // Stop TX when buffer is empty
  if (!RING_BUF_FILL(_port.tx_buf))
  {
    _port.im &= ~UART011_TXIM;
    _write(_port.im, UART011_IMSC);
  }
}

static void _rx_chars(void)
{
  // Local variables
  unsigned space = RING_BUF_SPACE(_port.rx_buf);
  unsigned status;
  unsigned ch;

  // Read from UART FIFO until it is empty or buffer is full
  while (space--)
  {
    // Check if UART FIFO empty
    status = _read(UART01x_FR);
    if (status & UART01x_FR_RXFE)
      break;

    // Read char from FIFO and update status
    ch = _read(UART01x_DR);

    // Record errors
    if (ch & UART011_DR_BE)
      _port.brk_errs++;
    if (ch & UART011_DR_PE)
      _port.parity_errs++;
    if (ch & UART011_DR_FE)
      _port.frame_errs++;
    if (ch & UART011_DR_OE)
      _port.overruns++;

    // Queue char
    RING_BUF_PUSH(_port.rx_buf, ch);
  }

  // Stop RX interrupts when buffer is full
  if (!RING_BUF_SPACE(_port.rx_buf))
  {
    _port.im &= ~(UART011_RXIM | UART011_RTIM);
    _write(_port.im, UART011_IMSC);
  }
}

static int _read_status(void)
{
  int status = 0;
  unsigned fr = _read(UART01x_FR);
  unsigned cr = _read(UART011_CR);

  if (fr & UART01x_FR_DSR)
    status |= TIOCM_LE; // DSR (data set ready)
  if (cr & UART011_CR_DTR)
    status |= TIOCM_DTR; // DTR (data terminal ready)
  if (cr & UART011_CR_RTS)
    status |= TIOCM_RTS; // RTS (request to send)
  // TODO What is TIOCM_ST - Secondary TXD (transmit)?
  // TODO What is TIOCM_SR - Secondary RXD (receive)?
  if (fr & UART01x_FR_CTS)
    status |= TIOCM_CTS; // CTS (clear to send)
  if (fr & UART01x_FR_DCD)
    status |= TIOCM_CD; // DCD (data carrier detect)
  if (fr & UART011_FR_RI)
    status |= TIOCM_RI; // RI  (ring)
  if (fr & UART01x_FR_DSR)
    status |= TIOCM_DSR; // DSR (data set ready)

  if (debug)
    printk(KERN_INFO "bbserial: _read_status() = %d\n", status);

  return status;
}

static void _write_status(int status)
{
  unsigned long flags;
  unsigned cr;

  if (debug)
    printk(KERN_INFO "bbserial: _write_status() = %d\n", status);

  spin_lock_irqsave(&_port.lock, flags);

  cr = _read(UART011_CR);

  // DTR (data terminal ready)
  if (status & TIOCM_DTR)
    cr |= UART011_CR_DTR;
  else
    cr &= ~UART011_CR_DTR;

  // RTS (request to send)
  if (status & TIOCM_RTS)
    cr |= UART011_CR_RTS;
  else
    cr &= ~UART011_CR_RTS;

  _write(cr, UART011_CR);

  spin_unlock_irqrestore(&_port.lock, flags);
}

static struct ktermios *_get_term(void)
{
  unsigned lcrh = _read(UART011_LCRH);
  unsigned cr = _read(UART011_CR);
  unsigned brd;
  speed_t baud;
  unsigned cflag;

  // Baud rate
  brd = _read(UART011_IBRD) << 6 | _read(UART011_FBRD);
  baud = clk_get_rate(_port.clk) * 4 / brd;
  tty_termios_encode_baud_rate(&_port.term, baud, baud);

  // Data bits
  switch (lcrh & UART01x_LCRH_WLEN_bm)
  {
  case UART01x_LCRH_WLEN_5:
    cflag = CS5;
    break;
  case UART01x_LCRH_WLEN_6:
    cflag = CS6;
    break;
  case UART01x_LCRH_WLEN_7:
    cflag = CS7;
    break;
  default:
    cflag = CS8;
    break;
  }

  // Stop bits
  if (lcrh & UART01x_LCRH_STP2)
    cflag |= CSTOPB;

  // Parity
  if (lcrh & UART01x_LCRH_PEN)
  {
    cflag |= PARENB;

    if (!(UART01x_LCRH_EPS & lcrh))
      cflag |= PARODD;
    if (UART011_LCRH_SPS & lcrh)
      cflag |= CMSPAR;
  }

  // Hardware flow control
  if (cr & UART011_CR_CTSEN)
    cflag |= CRTSCTS;

  _port.term.c_cflag = cflag;

  return &_port.term;
}

static void _set_baud(speed_t baud)
{
  if (debug)
    printk(KERN_INFO "bbserial: baud=%d\n", baud);

  unsigned brd;

  brd = clk_get_rate(_port.clk) * 16 / baud;

  if ((brd & 3) == 3)
    brd = (brd >> 2) + 1; // Round up
  else
    brd >>= 2;

  _write(brd & 0x3f, UART011_FBRD);
  _write(brd >> 6, UART011_IBRD);
}

static int _set_term(struct ktermios *term)
{
  unsigned lcrh = UART01x_LCRH_FEN; // Enable FIFOs
  unsigned cflag = term->c_cflag;
  speed_t baud;
  unsigned cr;

  // Data bits
  switch (cflag & CSIZE)
  {
  case CS5:
    lcrh |= UART01x_LCRH_WLEN_5;
    break;
  case CS6:
    lcrh |= UART01x_LCRH_WLEN_6;
    break;
  case CS7:
    lcrh |= UART01x_LCRH_WLEN_7;
    break;
  default:
    lcrh |= UART01x_LCRH_WLEN_8;
    break;
  }

  // Stop bits
  if (cflag & CSTOPB)
    lcrh |= UART01x_LCRH_STP2;

  // Parity
  if (cflag & PARENB)
  {
    lcrh |= UART01x_LCRH_PEN;

    if (!(cflag & PARODD))
      lcrh |= UART01x_LCRH_EPS;
    if (cflag & CMSPAR)
      lcrh |= UART011_LCRH_SPS;
  }

  // Get baud rate
  baud = tty_termios_baud_rate(term);

  // Set
  unsigned long flags;
  spin_lock_irqsave(&_port.lock, flags);

  // Hardware flow control
  cr = _read(UART011_CR);
  if (cflag & CRTSCTS)
    cr |= UART011_CR_CTSEN;

  _write(0, UART011_CR);      // Disable
  _set_baud(baud);            // Baud
  _write(lcrh, UART011_LCRH); // Must be after baud
  _write(cr, UART011_CR);     // Enable

  spin_unlock_irqrestore(&_port.lock, flags);

  return 0;
}

static void _flush_input(void)
{
  unsigned long flags;
  spin_lock_irqsave(&_port.lock, flags);

  RING_BUF_CLEAR(_port.rx_buf);

  spin_unlock_irqrestore(&_port.lock, flags);
}

static void _flush_output(void)
{
  unsigned long flags;
  spin_lock_irqsave(&_port.lock, flags);

  RING_BUF_CLEAR(_port.tx_buf);

  spin_unlock_irqrestore(&_port.lock, flags);
}

static irqreturn_t _interrupt(int irq, void *id)
{
  unsigned status;
  unsigned txSpace;

  status = _read(UART011_MIS);

  // RX interrupt
  if (status & (UART011_RXIS | UART011_RTIS))
    _rx_chars();

  // TX interrupt
  if (status & UART011_TXIS)
  {
    txSpace = RING_BUF_SPACE(_port.tx_buf);
    if (txSpace)
      _tx_chars();
  }

  // Clear interrupts
  _write(status, UART011_ICR);

  return IRQ_HANDLED;
}

static void _enable_tx(void)
{
  unsigned long flags;
  spin_lock_irqsave(&_port.lock, flags);

  _port.im |= UART011_TXIM;
  _write(_port.im, UART011_IMSC);
  _tx_chars(); // Must prime the pump

  spin_unlock_irqrestore(&_port.lock, flags);
}

static int _tx_enabled(void) { return _port.im & UART011_TXIM; }

static void _enable_rx(void)
{
  unsigned long flags;
  spin_lock_irqsave(&_port.lock, flags);

  _port.im |= UART011_RTIM | UART011_RXIM;
  _write(_port.im, UART011_IMSC);

  spin_unlock_irqrestore(&_port.lock, flags);
}

static int _rx_enabled(void) { return _port.im & (UART011_RTIM | UART011_RXIM); }

static int _dev_open(struct inode *inodep, struct file *filep)
{
  if (debug)
    printk(KERN_INFO "bbserial: open()\n");
  if (_port.open)
    return -EBUSY;
  _port.open = 1;
  return 0;
}

static ssize_t _dev_read(struct file *filep, char *buffer, size_t len,
                         loff_t *offset)
{
  ssize_t bytes = 0;
  unsigned fill = RING_BUF_FILL(_port.rx_buf);
  unsigned long flags;

  if (debug)
    printk(KERN_INFO "bbserial: read() len=%zu overruns=%d\n", len,
           _port.overruns);

  if (fill)
  {
    if (len < fill)
      fill = len;

    spin_lock_irqsave(&_port.lock, flags);

    while (fill--)
    {
      buffer[bytes++] = RING_BUF_PEEK(_port.rx_buf);
      RING_BUF_POP(_port.rx_buf);
    }

    // Start RX interrupts when buffer is not full
    if (RING_BUF_SPACE(_port.rx_buf))
      _enable_rx();

    spin_unlock_irqrestore(&_port.lock, flags);
  }

  return bytes;
}

static ssize_t _dev_write(struct file *filep, const char *buffer, size_t len,
                          loff_t *offset)
{
  ssize_t bytes = 0;
  unsigned space = RING_BUF_SPACE(_port.tx_buf);
  unsigned long flags;

  if (debug)
    printk(KERN_INFO "bbserial: write() len=%zu tx=%d rx=%d\n",
           len, RING_BUF_FILL(_port.tx_buf), RING_BUF_FILL(_port.rx_buf));

  if (space)
  {
    if (len < space)
      space = len;

    spin_lock_irqsave(&_port.lock, flags);

    while (space--)
      RING_BUF_PUSH(_port.tx_buf, buffer[bytes++]);

    // Start TX interrupts
    _enable_tx();

    spin_unlock_irqrestore(&_port.lock, flags);
  }

  return bytes;
}

static int _dev_release(struct inode *inodep, struct file *filep)
{
  printk(KERN_INFO "bbserial: release()\n");
  _port.open = 0;
  return 0;
}

static unsigned _dev_poll(struct file *file, poll_table *wait)
{
  if (debug)
  {
    unsigned events = poll_requested_events(wait);
    printk(KERN_INFO "bbserial: poll(in=%s, out=%s)\n",
           (events & POLLIN) ? "true" : "false",
           (events & POLLOUT) ? "true" : "false");
  }

  poll_wait(file, &_port.read_wait, wait);
  poll_wait(file, &_port.write_wait, wait);

  unsigned ret = 0;
  if (RING_BUF_FILL(_port.rx_buf))
    ret |= POLLIN | POLLRDNORM;
  if (RING_BUF_SPACE(_port.tx_buf))
    ret |= POLLOUT | POLLWRNORM;

  if (debug)
    printk(KERN_INFO "bbserial: tx=%d rx=%d\n",
           RING_BUF_FILL(_port.tx_buf), RING_BUF_FILL(_port.rx_buf));

  return ret;
}

static long _dev_ioctl(struct file *file, unsigned cmd, unsigned long arg)
{
  if (debug)
    printk(KERN_INFO "bbserial: ioctl() cmd=0x%04x arg=%lu\n", cmd, arg);

  int __user *ptr = (int __user *)arg;
  int status;
  int ret = 0;

  switch (cmd)
  {
  case TCGETS:
  { // Get serial port settings
    struct ktermios *term = _get_term();
    if (copy_to_user((void __user *)arg, &term, sizeof(struct termios)))
      return -EFAULT;
    return 0;
  }

  case TCSETS:
  { // Set serial port settings
    struct ktermios term;
    if (copy_from_user(&term, (void __user *)arg, sizeof(struct termios)))
      return -EFAULT;
    return _set_term(&term);
  }

  case TIOCMGET: // Get status of modem bits
    status = _read_status();
    if (put_user(status, ptr))
      ret = -EFAULT;
    break;

  case TIOCMSET: // Set status of modem bits
    if (get_user(status, ptr))
      ret = -EFAULT;
    else
      _write_status(status);
    break;

  case TIOCMBIC: // Clear indicated modem bits
    if (get_user(status, ptr))
      ret = -EFAULT;
    else
      _write_status(~status & _read_status());
    break;

  case TIOCMBIS: // Set indicated modem bits
    if (get_user(status, ptr))
      ret = -EFAULT;
    else
      _write_status(status | _read_status());
    break;

  case TCFLSH: // Flush
    if (arg == TCIFLUSH || arg == TCIOFLUSH)
      _flush_input();
    if (arg == TCOFLUSH || arg == TCIOFLUSH)
      _flush_output();
    return 0;

  case TIOCINQ:
    return put_user(RING_BUF_FILL(_port.rx_buf), ptr);
  case TIOCOUTQ:
    return put_user(RING_BUF_FILL(_port.tx_buf), ptr);

  default:
    return -ENOIOCTLCMD;
  }

  return ret;
}

static struct file_operations _ops = {
    .owner = THIS_MODULE,
    .open = _dev_open,
    .read = _dev_read,
    .write = _dev_write,
    .release = _dev_release,
    .poll = _dev_poll,  
    .unlocked_ioctl = _dev_ioctl,
};

static int _probe(struct platform_device *dev)
{
    struct resource *res;
    int ret;

    res = platform_get_resource(dev, IORESOURCE_MEM, 0);
    if (!res) {
        dev_err(&dev->dev, "No memory resource\n");
        return -ENODEV;
    }
    _port.base = devm_ioremap_resource(&dev->dev, res);
    if (IS_ERR(_port.base))
        return PTR_ERR(_port.base);

    ret = platform_get_irq(dev, 0);
    if (ret < 0)
        return ret;
    _port.irq = ret;

    // Request the IRQ
    ret = request_irq(_port.irq, _interrupt, IRQF_SHARED, 
                     "bbserial", &_port);
    if (ret) {
        dev_err(&dev->dev, "Cannot request IRQ\n");
        return ret;
    }

    return 0;
}

static int bbserial_remove(struct platform_device *dev)
{
    device_destroy(_port.class, MKDEV(_port.major, 0));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    class_destroy(_port.class);  // New API doesn't need release
#else
    class_destroy(_port.class);  // Old API with implicit release
#endif

    unregister_chrdev(_port.major, DEVICE_NAME);
    free_irq(_port.irq, &_port);
    
    return 0;
}

static struct platform_driver bbserial_driver = {
    .probe = _probe,
    .remove = bbserial_remove,
    .driver = {
        .name = "bbserial",
        .owner = THIS_MODULE,
    },
};

module_platform_driver(bbserial_driver);
