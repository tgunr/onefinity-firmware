import { writable } from 'svelte/store';

export const connectionStatus = writable('disconnected');
export const machineState = writable({});

let socket: WebSocket | null = null;

export function connectWebSocket() {
    if (socket) {
        socket.close();
    }

    // Using SockJS endpoint
    const wsUrl = `${window.location.protocol === 'https:' ? 'wss:' : 'ws:'}//${window.location.host}/sockjs/websocket`;
    socket = new WebSocket(wsUrl);

    socket.onopen = () => {
        connectionStatus.set('connected');
        console.log('WebSocket connected');
    };

    socket.onclose = () => {
        connectionStatus.set('disconnected');
        console.log('WebSocket disconnected');
        // Try to reconnect after 5 seconds
        setTimeout(connectWebSocket, 5000);
    };

    socket.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            if (data.state) {
                machineState.set(data.state);
            }
            // Handle other message types as needed
        } catch (e) {
            console.error('Error parsing WebSocket message:', e);
        }
    };

    socket.onerror = (error) => {
        console.error('WebSocket error:', error);
    };
}

export function sendCommand(command: string) {
    if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send(JSON.stringify(command));
    } else {
        console.error('WebSocket is not connected');
    }
}
