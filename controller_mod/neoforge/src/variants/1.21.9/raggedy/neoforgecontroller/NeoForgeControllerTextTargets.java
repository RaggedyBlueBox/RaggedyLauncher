package raggedy.neoforgecontroller;

import raggedy.neoforgecontroller.mixin.NeoForgeControllerBookAccessor;
import net.minecraft.client.gui.screens.Screen;
import net.minecraft.client.gui.screens.inventory.BookEditScreen;

final class NeoForgeControllerTextTargets {
    static void keyPress(net.minecraft.client.KeyboardHandler keyboard, long window, int key, int scan, int action, int modifiers) {
        ((raggedy.neoforgecontroller.mixin.NeoForgeControllerKeyboardAccessor) keyboard).raggedy$keyEvent(window, action, new net.minecraft.client.input.KeyEvent(key, scan, modifiers));
    }

    static Object bookTarget(Screen screen) {
        return screen instanceof BookEditScreen ? ((NeoForgeControllerBookAccessor) screen).raggedy$page() : null;
    }

    static boolean creativeSearchAllowed() {
        return true;
    }

    static boolean multiline(Screen screen) {
        return screen instanceof BookEditScreen;
    }
}
