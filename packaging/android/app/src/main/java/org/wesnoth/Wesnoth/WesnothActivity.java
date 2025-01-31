package org.wesnoth.Wesnoth;

import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import org.libsdl.app.SDLActivity;

public class WesnothActivity extends SDLActivity
{

// Needs to be inside an activity so we can use `startActivity`.
public void open(String url) {
	Log.d("WesnothActivity", "opening " + url);
	Intent openIntent = new Intent(Intent.ACTION_VIEW);
	openIntent.setData(Uri.parse(url));
	startActivity(openIntent);
}
}
