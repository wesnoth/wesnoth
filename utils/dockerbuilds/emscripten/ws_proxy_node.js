#!/usr/bin/env node
/**
 * Minimal WebSocket-to-TCP proxy for Wesnoth addon server.
 * Uses the 'ws' npm package for reliable WebSocket handling.
 *
 * URL format: ws://host:port/{target_host}/{target_port}
 * Example: ws://localhost:8041/add-ons.wesnoth.org/15019
 *
 * Usage: node ws_proxy_node.js [--port 8041] [-v]
 *
 * Requires: npm install ws
 */
const net = require('net');
const { WebSocketServer } = require('ws');

const args = process.argv.slice(2);
let port = 8041;
let verbose = false;
for (let i = 0; i < args.length; i++) {
  if (args[i] === '--port' && args[i+1]) port = parseInt(args[++i]);
  if (args[i] === '-v') verbose = true;
}

function log(...a) { if (verbose) console.log('[ws_proxy]', ...a); }

const wss = new WebSocketServer({ port, host: '127.0.0.1' });

wss.on('listening', () => {
  console.log(`WebSocket-to-TCP proxy listening on ws://127.0.0.1:${port}`);
  console.log(`URL format: ws://127.0.0.1:${port}/{target_host}/{target_port}`);
});

wss.on('connection', (ws, req) => {
  const path = req.url || '/';
  const parts = path.replace(/^\//, '').split('/');
  if (parts.length < 2) {
    log('Bad path:', path);
    ws.close();
    return;
  }

  const targetHost = parts[0];
  const targetPort = parseInt(parts[1]);
  log(`New connection → ${targetHost}:${targetPort}`);

  // Connect to TCP target
  const tcp = net.createConnection({ host: targetHost, port: targetPort }, () => {
    log(`TCP connected to ${targetHost}:${targetPort}`);
  });

  let alive = true;
  const cleanup = () => {
    if (!alive) return;
    alive = false;
    tcp.destroy();
    try { ws.close(); } catch(e) {}
    log('Connection closed');
  };

  // WebSocket → TCP
  ws.on('message', (data) => {
    const buf = Buffer.isBuffer(data) ? data : Buffer.from(data);
    log(`WS→TCP: ${buf.length} bytes`);
    tcp.write(buf);
  });

  // TCP → WebSocket
  tcp.on('data', (data) => {
    log(`TCP→WS: ${data.length} bytes`);
    if (ws.readyState === 1) { // OPEN
      ws.send(data);
    }
  });

  ws.on('error', (e) => { log('WS error:', e.message); cleanup(); });
  ws.on('close', cleanup);
  tcp.on('error', (e) => { log('TCP error:', e.message); cleanup(); });
  tcp.on('close', cleanup);
});

wss.on('error', (err) => {
  console.error('WebSocket server error:', err.message);
});
