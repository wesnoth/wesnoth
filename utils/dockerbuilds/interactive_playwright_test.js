#!/usr/bin/env node
/**
 * Interactive Playwright test for Wesnoth web build.
 * Takes multiple screenshots: initial load, after mouse movement, after clicking menus.
 */
const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright");

const URL = process.argv[2] || "http://127.0.0.1:8040/";
const OUT_DIR = process.argv[3] || "/tmp/wesnoth-interactive-test";

async function sleep(ms) {
  return new Promise(r => setTimeout(r, ms));
}

async function main() {
  fs.mkdirSync(OUT_DIR, { recursive: true });

  const browser = await chromium.launch({
    headless: true,
    args: ["--disable-dev-shm-usage", "--no-sandbox"],
  });

  const context = await browser.newContext({
    viewport: { width: 1280, height: 720 },
  });
  const page = await context.newPage();
  page.setDefaultTimeout(120000);

  const consoleLog = [];
  page.on("console", (msg) => {
    consoleLog.push(`[${msg.type()}] ${msg.text()}`);
  });

  console.log("1. Loading page...");
  await page.goto(URL, { waitUntil: "domcontentloaded", timeout: 120000 });

  // Wait for game to initialize and render title screen
  console.log("2. Waiting 25s for game init...");
  await sleep(25000);

  // Screenshot 1: Initial state
  console.log("3. Screenshot: initial state...");
  await page.screenshot({ path: path.join(OUT_DIR, "01-initial.png"), type: "png" });

  // Ensure canvas has focus
  console.log("4. Focusing canvas...");
  const canvas = page.locator("#canvas");
  const box = await canvas.boundingBox();
  console.log(`   Canvas bounding box: x=${box.x} y=${box.y} w=${box.width} h=${box.height}`);
  await canvas.click({ position: { x: 640, y: 360 } });
  await sleep(2000);
  await page.screenshot({ path: path.join(OUT_DIR, "02-after-focus.png"), type: "png" });

  // Based on probe results: clicking x=1130 y=560 opened Preferences.
  // Preferences is button 8 of 9 (near bottom). Buttons span ~y=280..620.
  // 9 buttons over ~340px => ~38px spacing, starting around y=280.
  // Campaigns (1st): y~280, Multiplayer (2nd): y~318, Load (3rd): y~356,
  // Add-ons (4th): y~394, Achievements (5th): y~432, Editor (6th): y~470,
  // Community (7th): y~508, Preferences (8th): y~546, Quit (9th): y~584

  // Click Campaigns (1st button, ~y=280)
  console.log("5. Clicking Campaigns...");
  await canvas.click({ position: { x: 1130, y: 280 } });
  await sleep(5000);
  await page.screenshot({ path: path.join(OUT_DIR, "03-campaigns.png"), type: "png" });

  // Close and click Multiplayer (2nd, ~y=318)
  console.log("6. Escape, then Multiplayer...");
  await page.keyboard.press("Escape");
  await sleep(2000);
  await canvas.click({ position: { x: 1130, y: 318 } });
  await sleep(5000);
  await page.screenshot({ path: path.join(OUT_DIR, "04-multiplayer.png"), type: "png" });

  // Close and click Map Editor (6th, ~y=470)
  console.log("7. Escape, then Map Editor...");
  await page.keyboard.press("Escape");
  await sleep(2000);
  await canvas.click({ position: { x: 1130, y: 470 } });
  await sleep(5000);
  await page.screenshot({ path: path.join(OUT_DIR, "05-editor.png"), type: "png" });

  // Close and click Preferences (8th, ~y=546) — known to work
  console.log("8. Escape, then Preferences...");
  await page.keyboard.press("Escape");
  await sleep(2000);
  await canvas.click({ position: { x: 1130, y: 546 } });
  await sleep(5000);
  await page.screenshot({ path: path.join(OUT_DIR, "06-preferences.png"), type: "png" });

  // Write rendering-related console lines
  const renderLines = consoleLog.filter(l =>
    /fallback|render texture|render buffer|logical|pixel scale|Setting mode|offscreen|integer/i.test(l)
  );
  console.log("\n=== Rendering-related console messages ===");
  for (const l of renderLines) console.log("  " + l);

  // Write full console log to file
  fs.writeFileSync(path.join(OUT_DIR, "console.log"), consoleLog.join("\n"));
  console.log(`\nTotal console messages: ${consoleLog.length}`);
  console.log(`Screenshots saved to: ${OUT_DIR}`);

  await context.close();
  await browser.close();
}

main().catch((err) => {
  console.error(String(err));
  process.exit(1);
});
