<script lang="ts">
  import { onMount } from 'svelte';
  import { connectionStatus, machineState, connectWebSocket, sendCommand } from './lib/stores/websocket';

  let jogDistance = 1.0;
  
  function jog(axis: string, direction: number) {
    sendCommand(`G91 G0 ${axis}${direction * jogDistance}`);
  }

  function home(axis: string) {
    sendCommand(`$home ${axis}`);
  }

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

      <div class="controls-section">
        <h2>Machine Controls</h2>
        
        <div class="jog-controls">
          <div class="jog-distance">
            <label>Jog Distance (mm)</label>
            <select bind:value={jogDistance}>
              <option value={0.1}>0.1</option>
              <option value={1.0}>1.0</option>
              <option value={10.0}>10.0</option>
              <option value={100.0}>100.0</option>
            </select>
          </div>

          <div class="axis-controls">
            <div class="axis">
              <h3>X Axis</h3>
              <button on:click={() => jog('X', -1)}>-X</button>
              <button on:click={() => home('x')}>Home X</button>
              <button on:click={() => jog('X', 1)}>+X</button>
            </div>

            <div class="axis">
              <h3>Y Axis</h3>
              <button on:click={() => jog('Y', -1)}>-Y</button>
              <button on:click={() => home('y')}>Home Y</button>
              <button on:click={() => jog('Y', 1)}>+Y</button>
            </div>

            <div class="axis">
              <h3>Z Axis</h3>
              <button on:click={() => jog('Z', -1)}>-Z</button>
              <button on:click={() => home('z')}>Home Z</button>
              <button on:click={() => jog('Z', 1)}>+Z</button>
            </div>
          </div>
        </div>
      </div>

      <div class="state-section">
        <h2>Machine State</h2>
        <div class="state-grid">
          {#if $machineState.state}
            <div class="state-item">
              <span class="label">State:</span>
              <span class="value">{$machineState.state}</span>
            </div>
          {/if}
          {#if $machineState.line}
            <div class="state-item">
              <span class="label">Line:</span>
              <span class="value">{$machineState.line}</span>
            </div>
          {/if}
          {#if $machineState.position}
            <div class="state-item">
              <span class="label">Position:</span>
              <span class="value">
                X: {$machineState.position.x?.toFixed(3) ?? 'N/A'},
                Y: {$machineState.position.y?.toFixed(3) ?? 'N/A'},
                Z: {$machineState.position.z?.toFixed(3) ?? 'N/A'}
              </span>
            </div>
          {/if}
        </div>
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

  h3 {
    color: #555;
    margin: 10px 0;
  }

  .status-section, .state-section, .controls-section {
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

  .jog-controls {
    display: flex;
    flex-direction: column;
    gap: 20px;
  }

  .jog-distance {
    display: flex;
    gap: 10px;
    align-items: center;
  }

  .jog-distance select {
    padding: 5px;
    border-radius: 4px;
    border: 1px solid #ccc;
  }

  .axis-controls {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    gap: 20px;
  }

  .axis {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
  }

  button {
    padding: 8px 16px;
    border: none;
    border-radius: 4px;
    background-color: #2196f3;
    color: white;
    cursor: pointer;
    font-weight: bold;
    transition: background-color 0.2s;
  }

  button:hover {
    background-color: #1976d2;
  }

  .state-grid {
    display: grid;
    gap: 10px;
  }

  .state-item {
    display: grid;
    grid-template-columns: 100px 1fr;
    gap: 10px;
    padding: 5px;
    border-bottom: 1px solid #eee;
  }

  .state-item .label {
    font-weight: bold;
    color: #666;
  }

  .state-item .value {
    color: #333;
  }
</style>
