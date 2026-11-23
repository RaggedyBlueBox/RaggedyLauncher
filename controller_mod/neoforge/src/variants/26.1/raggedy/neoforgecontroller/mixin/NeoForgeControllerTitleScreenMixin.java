package raggedy.neoforgecontroller.mixin;

import raggedy.controllercore.ControllerLog;
import raggedy.neoforgecontroller.NeoForgeControllerCompat;
import net.minecraft.client.gui.GuiGraphicsExtractor;
import net.minecraft.client.gui.screens.TitleScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.ModifyVariable;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(value = TitleScreen.class, remap = false)
public abstract class NeoForgeControllerTitleScreenMixin {
    private static boolean raggedy$playableLogged = false;

    @Inject(method = "extractRenderState", at = @At("HEAD"), remap = false)
    private void raggedy$updateControllerCursor(GuiGraphicsExtractor graphics, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        raggedy$logPlayableOnce();
        NeoForgeControllerCompat.updateScreenCursorBeforeRender((TitleScreen) (Object) this, mouseX, mouseY);
    }

    private static synchronized void raggedy$logPlayableOnce() {
        if (raggedy$playableLogged) {
            return;
        }
        raggedy$playableLogged = true;
        ControllerLog.log("xbox_compat", "raggedy:playable");
    }

    @ModifyVariable(method = "extractRenderState", at = @At("HEAD"), ordinal = 0, argsOnly = true, remap = false)
    private int raggedy$useControllerMouseX(int mouseX) {
        return NeoForgeControllerCompat.screenMouseX((TitleScreen) (Object) this, mouseX);
    }

    @ModifyVariable(method = "extractRenderState", at = @At("HEAD"), ordinal = 1, argsOnly = true, remap = false)
    private int raggedy$useControllerMouseY(int mouseY) {
        return NeoForgeControllerCompat.screenMouseY((TitleScreen) (Object) this, mouseY);
    }
}
