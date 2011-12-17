/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the NaCl-LICENSE file.
 */

function toArray(list) {
  return Array.prototype.slice.call(list || [], 0);
}

function errorHandler(e) {
  var msg = '';

  switch (e.code) {
    case FileError.QUOTA_EXCEEDED_ERR:
      msg = 'QUOTA_EXCEEDED_ERR';
      break;
    case FileError.NOT_FOUND_ERR:
      msg = 'NOT_FOUND_ERR';
      break;
    case FileError.SECURITY_ERR:
      msg = 'SECURITY_ERR';
      break;
    case FileError.INVALID_MODIFICATION_ERR:
      msg = 'INVALID_MODIFICATION_ERR';
      break;
    case FileError.INVALID_STATE_ERR:
      msg = 'INVALID_STATE_ERR';
      break;
    default:
      msg = 'Unknown Error';
      break;
  };

  console.log('Error: ' + msg);
  document.getElementById('wesnoth').postMessage("");
}

function listResults(entries) {
    s = ''
    entries.forEach(function(entry, i) {
      s += entry.name;
      s += '\n';
    });
    console.log('entries: ' + s);
    document.getElementById('wesnoth').postMessage(s);
}

function readDir(dir) {
  var dirReader = dir.createReader();
  var entries = [];

  // Call the reader.readEntries() until no more results are returned.
  var readEntries = function() {
     dirReader.readEntries (function(results) {
      if (!results.length) {
        listResults(entries.sort());
      } else {
        entries = entries.concat(toArray(results));
        readEntries();
      }
    }, errorHandler);
  };

  readEntries(); // Start reading dirs.
}


function HandlePepperMountMessage(data) {
    path = data[1];
    onInitFs = function(fs) {
	fs.root.getDirectory(path, {}, readDir, errorHandler);
    };
    window.webkitRequestFileSystem(window.PERSISTENT, 5*1024*1024 /*5MB*/, onInitFs, errorHandler);
}
