package raggedy.forgecontroller;

import raggedy.forgecontroller.mixin.ForgeControllerBookAccessor;
import net.minecraft.client.gui.screens.Screen;
import net.minecraft.client.gui.screens.inventory.BookEditScreen;

final class ForgeControllerTextTargets {
    static Object bookTarget(Screen screen) {
        if (!(screen instanceof BookEditScreen)) return null;
        ForgeControllerBookAccessor access = (ForgeControllerBookAccessor) screen;
        return access.raggedy$signing() ? access.raggedy$title() : access.raggedy$page();
    }

    static boolean multiline(Screen screen) {
        return screen instanceof BookEditScreen && !((ForgeControllerBookAccessor) screen).raggedy$signing();
    }
}
