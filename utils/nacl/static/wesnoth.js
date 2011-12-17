/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the NaCl-LICENSE file.
 */

function updateProgressBar(percent, message, styleBackground,
                           styleForeground, styleText) {
    console.log("message: " + message);
    console.log("progress: " + percent + "%");
    var progress_bar =
        document.getElementById('progress_bar');
    var ctx = progress_bar.getContext('2d');
    var width = progress_bar.width;
    ctx.fillStyle = styleForeground ? styleForeground :
        "#52565a";
    ctx.fillRect(0, 0, percent * width, 20);
    ctx.fillStyle = styleBackground ? styleBackground :
        "#ddccbb";
    ctx.fillRect(percent * width, 0, width, 20);
    ctx.fillStyle = styleText ? styleText : "black";
    ctx.textAlign = "center";
    ctx.textBaseline = "middle";
    ctx.font = 'sans-serif';
    ctx.fillText(message, width / 2, 10, 3 * width / 4);
}

function HandleProgress(event) {
    var loadPercent = 0.0;
    var loadPercentString;
    if (event.lengthComputable && event.total > 0) {
        loadPercent = event.loaded / event.total;
    } else {
        // The total length is not yet known.
        loadPercent = -1.0;
    }
    updateProgressBar(loadPercent, "Initializing Wesnoth (please be patient) ...");
}

function HandleMessage(event) {
    console.log(event.data);
    var data = JSON.parse(event.data);
    console.log(data)
    console.log(data[0])
    console.log(data[0] == 'ReadDirectory')
    if (data[0] == 'ReadDirectory') {
	HandlePepperMountMessage(data);
    } else {
	updateProgressBar(data[1] / data[2], data[0]);
    }
}

function updateStatusFail(message) {
    updateProgressBar(1, message, 'black',
                      'red', 'black')
}

function moduleLoadError() {
    var msg = "NaCl module load error: " + document.getElementById('wesnoth').lastError;
    updateStatusFail(msg);
    console.log(msg);
}

function moduleLoadAbort() {
    var msg = "NaCl module load aborted: " + document.getElementById('wesnoth').lastError;
    updateStatusFail(msg);
    console.log(msg);
}

function requestQuotaAndStartWesnoth() {
    quota_required = 380*1024*1024;
    window.webkitStorageInfo.requestQuota(PERSISTENT, quota_required, function(bytes) {
            console.log("Persistent storage quota granted: " + bytes + " bytes");
            if (bytes >= quota_required) {
                var embed = document.createElement('embed');
                embed.setAttribute('name', 'nacl_module');
                embed.setAttribute('id', 'wesnoth');
                embed.setAttribute('width', 1024);
                embed.setAttribute('height', 800);
                embed.setAttribute('src', 'wesnoth.nmf');
                embed.setAttribute('type', 'application/x-nacl');
                var div = document.getElementById("nacl_div");
                div.appendChild(embed);
                div.addEventListener('progress', HandleProgress, true);
                div.addEventListener('message', HandleMessage, true);
                div.addEventListener('error', moduleLoadError, true);
                div.addEventListener('abort', moduleLoadAbort, true);
            } else {
                updateStatusFail("Unsufficient HTML5 file system quota: " + bytes + " bytes");
                console.log("Unsufficient HTML5 file system quota: " + bytes + " bytes");
            }
        }, function(e) {
            updateStatusFail("HTML5 file system quota request failed");
            console.log("Quota request error: " + e);
        });
}


function checkBrowser() {
    var isValidBrowser = false;
    var browserSupportStatus = 0;
    var checker = new browser_version.BrowserChecker(15,  // Minumum Chrome version.
                                                     navigator["appVersion"],
                                                     navigator["plugins"]);
    checker.checkBrowser();
    isValidBrowser = checker.getIsValidBrowser();
    browserSupportStatus = checker.getBrowserSupportStatus();
    
    switch (browserSupportStatus) {
    case browser_version.BrowserChecker.StatusValues.NACL_ENABLED:
        console.log('Native Client plugin enabled.');
        break;
    case browser_version.BrowserChecker.StatusValues.UNKNOWN_BROWSER:
        updateStatusFail('UNKNOWN BROWSER');
        break;
    case browser_version.BrowserChecker.StatusValues.CHROME_VERSION_TOO_OLD:
        console.log('Chrome too old: You must use Chrome version 15 or later.');
        updateStatusFail('NEED CHROME 15 OR LATER');
        break;
    case browser_version.BrowserChecker.StatusValues.NACL_NOT_ENABLED:
        console.log(
                    'NaCl disabled: Native Client is not enabled.<br>' +
                    'Please go to <b>chrome://plugins</b> and enable Native Client ' +
                    'plugin.');
        updateStatusFail('NativeClient NOT ENABLED');
        break;
    case browser_version.BrowserChecker.StatusValues.NOT_USING_SERVER:
        console.log(
                    'file: URL detected, please use a web server to host Native ' +
                    'Client applications.');
        updateStatusFail('file:// URLs NOT ALLOWED');
    default:
        console.log('Unknown error: Unable to detect browser and/or ' +
                    'Native Client support.');
        updateStatusFail('UNKNOWN ERROR');
        break;
    }
    return isValidBrowser && browserSupportStatus == browser_version.BrowserChecker.StatusValues.NACL_ENABLED;
}

if (checkBrowser())
    requestQuotaAndStartWesnoth();

