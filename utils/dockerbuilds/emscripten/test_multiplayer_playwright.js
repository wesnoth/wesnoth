/**
 * Playwright test: verify Wesnoth web multiplayer connects to wesnothd.
 * Used inside Docker container with serve_coi.py and ws_proxy_node.js running.
 *
 * Usage: NODE_PATH=/path/to/node_modules node test_multiplayer_playwright.js [url] [screenshot_dir]
 */
const { chromium } = require('playwright');
const fs = require('fs');

const URL = process.argv[2] || 'http://127.0.0.1:8040/';
const SDIR = process.argv[3] || '/screenshots';

fs.mkdirSync(SDIR, { recursive: true });

function sleep(ms) { return new Promise(r => setTimeout(r, ms)); }

async function screenshot(page, name) {
  const path = `${SDIR}/${name}.png`;
  await page.screenshot({ path });
  console.log(`  screenshot: ${name}`);
  return path;
}

async function waitForCanvas(page, timeoutMs) {
  // Use a temporary 2D canvas + drawImage to read pixels non-destructively.
  // Do NOT call canvas.getContext('webgl2') — that steals SDL's WebGL context.
  const start = Date.now();
  while (Date.now() - start < timeoutMs) {
    const ready = await page.evaluate(() => {
      const c = document.getElementById('canvas');
      if (!c || c.width === 0 || c.height === 0) return false;
      try {
        const tmp = document.createElement('canvas');
        tmp.width = c.width;
        tmp.height = c.height;
        const ctx = tmp.getContext('2d', { willReadFrequently: true });
        if (!ctx) return false;
        ctx.drawImage(c, 0, 0);
        const d = ctx.getImageData(Math.floor(c.width/2), Math.floor(c.height/2), 1, 1).data;
        return d[3] !== 0 && (d[0] + d[1] + d[2]) > 30;
      } catch(e) { return false; }
    });
    if (ready) {
      console.log(`  Canvas ready after ${((Date.now()-start)/1000).toFixed(1)}s`);
      return true;
    }
    await sleep(2000);
  }
  return false;
}

(async () => {
  console.log('Launching browser...');
  const browser = await chromium.launch({
    headless: true,
    args: ['--no-sandbox', '--disable-dev-shm-usage',
           '--enable-features=WebAssemblyJSPromiseIntegration',
           '--enable-unsafe-swiftshader'],
  });
  const context = await browser.newContext({ viewport: { width: 1280, height: 720 } });
  const page = await context.newPage();
  page.setDefaultTimeout(120000);

  const logs = [];
  page.on('console', msg => {
    const text = msg.text();
    logs.push({ type: msg.type(), text });
    if (text.includes('WebSocket') || text.includes('error') || text.includes('Error') ||
        text.includes('network') || text.includes('Setting mode') ||
        text.includes('postRun') || text.includes('handshake') || text.includes('transfer') ||
        text.includes('connect') || text.includes('WS ') || text.includes('wesnothd') ||
        text.includes('multiplayer') || text.includes('lobby')) {
      console.log(`  [${msg.type()}] ${text.substring(0, 250)}`);
    }
  });

  try {
    console.log('\n=== Step 1: Load game ===');
    await page.goto(URL, { waitUntil: 'domcontentloaded', timeout: 30000 });
    console.log('  Page loaded');

    console.log('\n=== Step 2: Wait for title screen (up to 120s) ===');
    const loaded = await waitForCanvas(page, 120000);
    if (!loaded) {
      await screenshot(page, '01-timeout');
      throw new Error('Canvas never rendered');
    }
    await sleep(5000);
    await screenshot(page, '01-title-screen');
    console.log('  Title screen rendered');

    // Known button positions at 1280x720:
    //   Campaign: (1160, 190), Multiplayer: (1160, 250), Load: (1160, 290)
    //   Add-ons: (1160, 390)

    console.log('\n=== Step 3: Click Multiplayer button ===');
    console.log('  Clicking Multiplayer at (1160, 250)...');
    await page.mouse.click(1160, 250);
    await sleep(3000);
    await screenshot(page, '02-multiplayer-menu');

    console.log('\n=== Step 4: Click Join Official Server ===');
    // Multiplayer menu at 1280x720 has 4 options in a row:
    //   Join Official Server (~370,430), Connect to Server (~530,430),
    //   Host Networked Game (~680,430), Local Game (~830,430)
    const preConnectLogs = logs.length;
    console.log('  Double-clicking Join Official Server at (370, 430)...');
    await page.mouse.dblclick(370, 430);
    await sleep(3000);
    await screenshot(page, '03-after-server-select');

    // Check if we see a connection dialog or if there's a username prompt
    // The game may show a "login" screen or go directly to connecting
    console.log('\n=== Step 5: Wait for connection ===');
    // After server selection, the game connects through WebSocket proxy.
    // Wait for network activity.
    await sleep(15000);
    await screenshot(page, '04-connecting');

    // Check for network activity logs
    const postConnectLogs = logs.slice(preConnectLogs);
    const networkActivity = postConnectLogs.filter(l =>
      l.text.includes('WebSocket') || l.text.includes('WS ') ||
      l.text.includes('handshake') || l.text.includes('connect') ||
      l.text.includes('wesnothd') || l.text.includes('error'));
    console.log(`\n  Network activity after server select (${networkActivity.length} logs):`);
    networkActivity.forEach(l => console.log(`    [${l.type}] ${l.text.substring(0, 200)}`));

    // Wait longer for potential lobby
    await sleep(15000);
    await screenshot(page, '05-lobby-or-result');

    // Print all network-related logs
    const networkLogs = logs.filter(l =>
      l.text.includes('WebSocket') || l.text.includes('handshake') ||
      l.text.includes('WS ') || l.text.includes('transfer') ||
      l.text.includes('connect') || l.text.includes('wesnothd') ||
      l.text.includes('network') || l.text.includes('lobby'));
    console.log(`\n  All network logs (${networkLogs.length}):`);
    networkLogs.forEach(l => console.log(`    [${l.type}] ${l.text.substring(0, 250)}`));

    // Print errors
    const errors = logs.filter(l => l.type === 'error');
    if (errors.length > 0) {
      console.log(`\n  Errors (${errors.length}):`);
      errors.forEach(l => console.log(`    ${l.text.substring(0, 250)}`));
    }

    // Dump last 40 console logs
    console.log('\n  Last 40 console logs:');
    logs.slice(-40).forEach(l => console.log(`    [${l.type}] ${l.text.substring(0, 200)}`));

    console.log('\n=== Done ===');

  } catch(err) {
    console.error('Test error:', err.message);
    await screenshot(page, 'error-state');
    console.log('\n  Last 20 console logs:');
    logs.slice(-20).forEach(l => console.log(`    [${l.type}] ${l.text.substring(0, 200)}`));
  } finally {
    await browser.close();
  }
})();
