<script lang="ts">
  import { onMount } from 'svelte';
  import { connectionStatus, machineState, connectWebSocket } from '$lib/stores/websocket';

  onMount(() => {
    connectWebSocket();
  });
</script>

<main>
  <div class="content">
    <h1>Onefinity Controller</h1>
    
    <div class="status-section">
      <h2>Connection Status</h2>
      <p class="status {$connectionStatus}">
        {$connectionStatus}
      </p>
    </div>

    <div class="state-section">
      <h2>Machine State</h2>
      <pre>{JSON.stringify($machineState, null, 2)}</pre>
    </div>
  </div>
</main>

<style>
  main {
    font-family: Arial, sans-serif;
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100vh;
    margin: 0;
    background-color: #f0f0f0;
    padding: 20px;
  }
  
  .content {
    text-align: left;
    padding: 20px;
    background-color: white;
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    max-width: 800px;
    width: 100%;
  }

  h1 {
    text-align: center;
    margin-bottom: 30px;
  }

  .status-section, .state-section {
    margin-bottom: 20px;
  }

  .status {
    padding: 8px 16px;
    border-radius: 4px;
    display: inline-block;
  }

  .status.connected {
    background-color: #4caf50;
    color: white;
  }

  .status.disconnected {
    background-color: #f44336;
    color: white;
  }

  pre {
    background-color: #f5f5f5;
    padding: 15px;
    border-radius: 4px;
    overflow-x: auto;
  }
</style>
