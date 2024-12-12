<script lang="ts">
  import { onMount } from 'svelte';
  import { connectionStatus, machineState, connectWebSocket } from './lib/stores/websocket';

  onMount(() => {
    connectWebSocket();
  });
</script>

<div class="app">
  <main>
    <div class="content">
      <h1>Onefinity Controller</h1>
      
      <div class="status-section">
        <h2>Connection Status</h2>
        <p class="status-badge {$connectionStatus}">
          {$connectionStatus}
        </p>
      </div>

      <div class="state-section">
        <h2>Machine State</h2>
        <pre>{JSON.stringify($machineState, null, 2)}</pre>
      </div>
    </div>
  </main>
</div>

<style>
  :global(body) {
    margin: 0;
    padding: 0;
    font-family: Arial, sans-serif;
  }

  .app {
    width: 100%;
    min-height: 100vh;
    background-color: #f0f0f0;
  }

  main {
    display: flex;
    justify-content: center;
    align-items: flex-start;
    padding: 20px;
    min-height: 100vh;
    box-sizing: border-box;
  }
  
  .content {
    background-color: white;
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    padding: 20px;
    max-width: 800px;
    width: 100%;
  }

  h1 {
    text-align: center;
    margin-bottom: 30px;
    color: #333;
  }

  h2 {
    color: #444;
    margin-bottom: 15px;
  }

  .status-section, .state-section {
    margin-bottom: 20px;
    padding: 15px;
    background-color: #f8f8f8;
    border-radius: 6px;
  }

  .status-badge {
    display: inline-block;
    padding: 8px 16px;
    border-radius: 4px;
    font-weight: bold;
    text-transform: uppercase;
  }

  .status-badge.connected {
    background-color: #4caf50;
    color: white;
  }

  .status-badge.disconnected {
    background-color: #f44336;
    color: white;
  }

  .status-badge.error {
    background-color: #ff9800;
    color: white;
  }

  pre {
    background-color: #f5f5f5;
    padding: 15px;
    border-radius: 4px;
    overflow-x: auto;
    margin: 0;
    font-size: 14px;
    line-height: 1.4;
  }
</style>
