package raggedy.neoforgecontroller;

import raggedy.neoforgecontroller.mixin.NeoForgeControllerBookAccessor;
import net.minecraft.client.gui.screens.Screen;
import net.minecraft.client.gui.screens.inventory.BookEditScreen;

final class NeoForgeControllerTextTargets {
    static void keyPress(net.minecraft.client.KeyboardHandler keyboard, long window, int key, int scan, int action, int modifiers) {
        ((raggedy.neoforgecontroller.mixin.NeoForgeControllerKeyboardAccessor) keyboard).raggedy$keyPress(window, key, scan, action, modifiers);
    }

    static Object bookTarget(Screen screen) {
        if (!(screen instanceof BookEditScreen)) return null;
        NeoForgeControllerBookAccessor access = (NeoForgeControllerBookAccessor) screen;
        return access.raggedy$signing() ? access.raggedy$title() : access.raggedy$page();
    }

    static boolean creativeSearchAllowed() {
        return true;
    }

    static boolean multiline(Screen screen) {
        return screen instanceof BookEditScreen && !((NeoForgeControllerBookAccessor) screen).raggedy$signing();
    }
}
