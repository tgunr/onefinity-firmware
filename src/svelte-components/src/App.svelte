<script lang="ts">
  let time = new Date().toLocaleString();
  
  setInterval(() => {
    time = new Date().toLocaleString();
  }, 1000);
</script>

<main>
  <div class="content">
    <h1>Onefinity Controller</h1>
    <p>Svelte app loaded successfully!</p>
    <p>Current time: {time}</p>
  </div>
</main>

<style>
  main {
    font-family: Arial, sans-serif;
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh;
    margin: 0;
    background-color: #f0f0f0;
  }
  
  .content {
    text-align: center;
    padding: 20px;
    background-color: white;
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  }
</style>
