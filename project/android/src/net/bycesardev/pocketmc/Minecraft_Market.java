package net.bycesardev.pocketmc;

import android.content.Intent;
import android.net.Uri;

public class Minecraft_Market extends MainActivity {
	@Override
	public void buyGame() {
		final Uri buyLink = Uri.parse("market://details?id=com.mojang.minecraftpe");
        Intent marketIntent = new Intent( Intent.ACTION_VIEW, buyLink );
        startActivity(marketIntent);
	}
}
