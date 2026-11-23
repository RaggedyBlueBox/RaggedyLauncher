package raggedy.forgecontroller.mixin;

import raggedy.controllercore.ControllerLog;
import raggedy.forgecontroller.ForgeControllerCompat;
import net.minecraft.client.gui.GuiGraphics;
import net.minecraft.client.gui.screens.TitleScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.ModifyVariable;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(value = TitleScreen.class, remap = false)
public abstract class ForgeControllerTitleScreenMixin {
    private static boolean raggedy$playableLogged = false;

    @Inject(method = "m_88315_", at = @At("HEAD"), remap = false)
    private void raggedy$updateControllerCursor(GuiGraphics graphics, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        raggedy$logPlayableOnce();
        ForgeControllerCompat.updateScreenCursorBeforeRender((TitleScreen) (Object) this, mouseX, mouseY);
    }

    private static synchronized void raggedy$logPlayableOnce() {
        if (raggedy$playableLogged) {
            return;
        }
        raggedy$playableLogged = true;
        ControllerLog.log("xbox_compat", "raggedy:playable");
    }

    @ModifyVariable(method = "m_88315_", at = @At("HEAD"), ordinal = 0, argsOnly = true, remap = false)
    private int raggedy$useControllerMouseX(int mouseX) {
        return ForgeControllerCompat.screenMouseX(mouseX);
    }

    @ModifyVariable(method = "m_88315_", at = @At("HEAD"), ordinal = 1, argsOnly = true, remap = false)
    private int raggedy$useControllerMouseY(int mouseY) {
        return ForgeControllerCompat.screenMouseY(mouseY);
    }
}
