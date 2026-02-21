#!/usr/bin/env node

const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright");

async function withTimeout(promise, timeoutMs, label) {
  let timeoutId = null;
  try {
    return await Promise.race([
      promise,
      new Promise((_, reject) => {
        timeoutId = setTimeout(
          () => reject(new Error(`${label} timed out after ${timeoutMs}ms`)),
          timeoutMs
        );
      }),
    ]);
  } finally {
    if (timeoutId !== null) {
      clearTimeout(timeoutId);
    }
  }
}

function ensureParent(filePath) {
  fs.mkdirSync(path.dirname(filePath), { recursive: true });
}

function writeJson(filePath, value) {
  ensureParent(filePath);
  fs.writeFileSync(filePath, JSON.stringify(value, null, 2));
}

function classifyResult(consoleEvents, canvasProbe, stateText) {
  const joined = consoleEvents
    .map((e) => String(e.text || ""))
    .join("\n");

  if (/createShader/.test(joined)) {
    return "FAIL_CREATE_SHADER";
  }
  if (/InvalidStateError/.test(joined) && /offscreen/i.test(joined)) {
    return "FAIL_OFFSCREEN_CONTEXT";
  }
  if (/LZ4 not present/i.test(joined)) {
    return "FAIL_LZ4_MISMATCH";
  }
  if (/postRun/.test(joined) && !/Setting mode to/.test(joined)) {
    return "FAIL_POSTRUN_EARLY";
  }
  if (canvasProbe && canvasProbe.ready && canvasProbe.non > 0) {
    return "PASS_VISIBLE_PIXELS";
  }
  if (/Setting mode to/.test(joined)) {
    return "PARTIAL_MODE_SET_NO_VISIBLE_PIXELS";
  }
  if (typeof stateText === "string" && stateText.includes("onExit")) {
    return "FAIL_EXITED";
  }
  return "UNKNOWN";
}

async function main() {
  const [
    ,
    ,
    url,
    consoleOutPath,
    resultOutPath,
    screenshotOutPath,
    waitMsArg,
  ] = process.argv;

  if (!url || !consoleOutPath || !resultOutPath || !screenshotOutPath) {
    console.error(
      "Usage: run_playwright_check.js <url> <consoleOutPath> <resultOutPath> <screenshotOutPath> [waitMs]"
    );
    process.exit(2);
  }

  const waitMs = Number.isFinite(Number(waitMsArg)) ? Number(waitMsArg) : 15000;
  const consoleEvents = [];
  const result = {
    status: "ok",
    state: null,
    canvasProbe: null,
    classification: "UNKNOWN",
    screenshot: "ok",
    waitMs,
    steps: {},
  };

  const browser = await chromium.launch({
    headless: true,
    args: ["--disable-dev-shm-usage", "--no-sandbox"],
  });

  try {
    const context = await browser.newContext({
      viewport: { width: 1365, height: 768 },
    });
    const page = await context.newPage();
    page.setDefaultTimeout(120000);

    page.on("console", (msg) => {
      consoleEvents.push({
        type: msg.type(),
        text: msg.text(),
      });
    });

    page.on("pageerror", (err) => {
      consoleEvents.push({
        type: "pageerror",
        text: String(err),
      });
    });

    page.on("requestfailed", (req) => {
      consoleEvents.push({
        type: "requestfailed",
        text: `${req.url()} :: ${req.failure()?.errorText || "unknown"}`,
      });
    });

    try {
      await withTimeout(
        page.goto(url, { waitUntil: "domcontentloaded", timeout: 120000 }),
        120000,
        "goto"
      );
      result.steps.goto = "ok";
    } catch (err) {
      result.steps.goto = `error: ${String(err)}`;
    }

    try {
      await withTimeout(page.waitForTimeout(waitMs), waitMs + 5000, "wait");
      result.steps.wait = "ok";
    } catch (err) {
      result.steps.wait = `error: ${String(err)}`;
    }

    try {
      result.state = await withTimeout(
        page.evaluate(() => {
          if (typeof window.render_game_to_text === "function") {
            return window.render_game_to_text();
          }
          return null;
        }),
        30000,
        "evaluate(render_game_to_text)"
      );
      result.steps.evaluate = "ok";
    } catch (err) {
      result.state = `state_error: ${String(err)}`;
      result.steps.evaluate = `error: ${String(err)}`;
    }

    try {
      result.canvasProbe = await withTimeout(
        page.evaluate(() => {
          const canvas =
            document.getElementById("canvas") ||
            document.querySelector("canvas");
          if (!canvas) {
            return { ready: false, reason: "no-canvas" };
          }
          const w = canvas.width || 0;
          const h = canvas.height || 0;
          if (w <= 0 || h <= 0) {
            return {
              ready: false,
              reason: "zero-size",
              width: w,
              height: h,
              clientWidth: canvas.clientWidth || 0,
              clientHeight: canvas.clientHeight || 0,
            };
          }
          // Try 2D context first; if canvas is WebGL, copy to a temp canvas
          let ctx = canvas.getContext("2d", { willReadFrequently: true });
          if (!ctx) {
            // WebGL canvas — draw onto a temporary 2D canvas to read pixels
            const tmp = document.createElement("canvas");
            tmp.width = w;
            tmp.height = h;
            ctx = tmp.getContext("2d", { willReadFrequently: true });
            if (!ctx) {
              return {
                ready: false,
                reason: "no-2d-context",
                width: w,
                height: h,
                clientWidth: canvas.clientWidth || 0,
                clientHeight: canvas.clientHeight || 0,
              };
            }
            ctx.drawImage(canvas, 0, 0);
          }

          const stepX = Math.max(1, Math.floor(w / 48));
          const stepY = Math.max(1, Math.floor(h / 28));
          let non = 0;
          let total = 0;
          for (let y = 0; y < h; y += stepY) {
            for (let x = 0; x < w; x += stepX) {
              const d = ctx.getImageData(x, y, 1, 1).data;
              if (d[3] !== 0 && (d[0] !== 0 || d[1] !== 0 || d[2] !== 0)) {
                non++;
              }
              total++;
            }
          }
          const c = ctx.getImageData(
            Math.floor(w / 2),
            Math.floor(h / 2),
            1,
            1
          ).data;
          return {
            ready: true,
            width: w,
            height: h,
            clientWidth: canvas.clientWidth || 0,
            clientHeight: canvas.clientHeight || 0,
            non,
            total,
            center: [c[0], c[1], c[2], c[3]],
          };
        }),
        30000,
        "evaluate(canvasProbe)"
      );
      result.steps.canvasProbe = "ok";
    } catch (err) {
      result.canvasProbe = { ready: false, error: String(err) };
      result.steps.canvasProbe = `error: ${String(err)}`;
    }

    try {
      ensureParent(screenshotOutPath);
      await withTimeout(
        page.screenshot({
          path: screenshotOutPath,
          type: "png",
          timeout: 30000,
        }),
        35000,
        "screenshot"
      );
      result.steps.screenshot = "ok";
    } catch (err) {
      result.screenshot = `error: ${String(err)}`;
      result.steps.screenshot = `error: ${String(err)}`;
    }

    try {
      await withTimeout(context.close(), 5000, "context.close");
      result.steps.contextClose = "ok";
    } catch (err) {
      result.steps.contextClose = `error: ${String(err)}`;
    }
  } catch (err) {
    result.status = "error";
    result.error = String(err);
  } finally {
    try {
      await withTimeout(browser.close(), 5000, "browser.close");
    } catch (_) {
      // Ignore close timeout; result artifacts are already written below.
    }
  }

  result.classification = classifyResult(
    consoleEvents,
    result.canvasProbe,
    result.state
  );

  writeJson(consoleOutPath, consoleEvents);
  writeJson(resultOutPath, result);

  console.log(JSON.stringify(result));
}

main().catch((err) => {
  console.error(String(err));
  process.exit(1);
});
