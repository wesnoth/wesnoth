/**
 * Playwright test: verify Wesnoth web local hotseat game starts.
 * No network required — this is purely local multiplayer.
 *
 * Usage: NODE_PATH=/path/to/node_modules node test_hotseat_playwright.js [url] [screenshot_dir]
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
    if (text.includes('error') || text.includes('Error') ||
        text.includes('Setting mode') || text.includes('postRun') ||
        text.includes('hotseat') || text.includes('multiplayer') ||
        text.includes('game_') || text.includes('scenario')) {
      console.log(`  [${msg.type()}] ${text.substring(0, 250)}`);
    }
  });

  try {
    console.log('\n=== Step 1: Load game ===');
    await page.goto(URL, { waitUntil: 'domcontentloaded', timeout: 30000 });

    console.log('\n=== Step 2: Wait for title screen ===');
    const loaded = await waitForCanvas(page, 120000);
    if (!loaded) throw new Error('Canvas never rendered');
    await sleep(5000);
    await screenshot(page, '01-title-screen');

    console.log('\n=== Step 3: Click Multiplayer ===');
    await page.mouse.click(1160, 250);
    await sleep(3000);
    await screenshot(page, '02-multiplayer-menu');

    console.log('\n=== Step 4: Double-click Local Game ===');
    // Local Game is the rightmost option at ~(830, 430)
    await page.mouse.dblclick(830, 430);
    await sleep(5000);
    await screenshot(page, '03-local-game-setup');

    console.log('\n=== Step 5: Interact with game setup ===');
    // After selecting Local Game, we should see a game creation dialog
    // Take a screenshot to see what's there
    await sleep(5000);
    await screenshot(page, '04-game-config');

    // Try clicking OK/Create Game — typically at bottom center
    console.log('  Clicking Create Game / OK...');
    await page.mouse.click(640, 680);
    await sleep(5000);
    await screenshot(page, '05-after-create');

    // Print errors
    const errors = logs.filter(l => l.type === 'error');
    if (errors.length > 0) {
      console.log(`\n  Errors (${errors.length}):`);
      errors.forEach(l => console.log(`    ${l.text.substring(0, 250)}`));
    }

    console.log('\n  Last 30 console logs:');
    logs.slice(-30).forEach(l => console.log(`    [${l.type}] ${l.text.substring(0, 200)}`));

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
