################################################################################
#                                                                              #
#                This file is part of the Buildbotics firmware.                #
#                                                                              #
#                  Copyright (c) 2015 - 2018, Buildbotics LLC                  #
#                             All rights reserved.                             #
#                                                                              #
#     This file ("the software") is free software: you can redistribute it     #
#     and/or modify it under the terms of the GNU General Public License,      #
#      version 2 as published by the Free Software Foundation. You should      #
#      have received a copy of the GNU General Public License, version 2       #
#     along with the software. If not, see <http://www.gnu.org/licenses/>.     #
#                                                                              #
#     The software is distributed in the hope that it will be useful, but      #
#          WITHOUT ANY WARRANTY; without even the implied warranty of          #
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       #
#               Lesser General Public License for more details.                #
#                                                                              #
#       You should have received a copy of the GNU Lesser General Public       #
#                License along with the software.  If not, see                 #
#                       <http://www.gnu.org/licenses/>.                        #
#                                                                              #
#                For information regarding this software email:                #
#                  "Joseph Coffland" <joseph@buildbotics.com>                  #
#                                                                              #
################################################################################

import time


def read_temp():
    # Return mock temperature value
    return 45


def set_max_freq(freq):
    # Mock implementation
    pass


class MonitorTemp(object):
    def __init__(self, app):
        self.app = app
        self.last_temp = 0
        self.freq = 600000
        self.enabled = True
        self.callback()


    def callback(self):
        if not self.enabled: return
        
        try:
            temp = read_temp()
            if self.last_temp != temp:
                self.app.get_ctrl().configure([{'temp': temp}])
                self.last_temp = temp

        except Exception as e:
            self.app.get_log().warning('Temperature status: %s' % e)

        # Reschedule
        self.app.get_ioloop().call_later(60, self.callback)
