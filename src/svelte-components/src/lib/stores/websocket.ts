import { writable } from 'svelte/store';
import SockJS from 'sockjs-client';

export const connectionStatus = writable('disconnected');
export const machineState = writable({});

let socket: WebSocket | null = null;

export function connectWebSocket() {
    if (socket) {
        socket.close();
    }

    // Using SockJS endpoint
    console.log('Connecting to SockJS endpoint');
    socket = new SockJS('/sockjs');

    socket.onopen = () => {
        connectionStatus.set('connected');
        console.log('WebSocket connected successfully');
    };

    socket.onclose = (event) => {
        connectionStatus.set('disconnected');
        console.log('WebSocket closed with code:', event.code, 'reason:', event.reason);
        // Try to reconnect after 5 seconds
        setTimeout(connectWebSocket, 5000);
    };

    socket.onmessage = (event) => {
        try {
            console.log('Received WebSocket message:', event.data);
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
        connectionStatus.set('error');
    };
}

export function sendCommand(command: string) {
    if (socket && socket.readyState === SockJS.OPEN) {
        socket.send(JSON.stringify(command));
    } else {
        console.error('WebSocket is not connected');
    }
}
