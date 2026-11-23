package raggedy.neoforgecontroller.mixin;

import raggedy.neoforgecontroller.NeoForgeControllerCompat;
import net.minecraft.client.gui.GuiGraphicsExtractor;
import net.minecraft.client.gui.screens.Screen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.ModifyVariable;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(value = Screen.class, remap = false)
public abstract class NeoForgeControllerScreenMixin {
    @Inject(method = "extractRenderState", at = @At("HEAD"), remap = false)
    private void raggedy$updateControllerCursor(GuiGraphicsExtractor graphics, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        NeoForgeControllerCompat.updateScreenCursorBeforeRender((Screen) (Object) this, mouseX, mouseY);
    }

    @ModifyVariable(method = "extractRenderState", at = @At("HEAD"), ordinal = 0, argsOnly = true, remap = false)
    private int raggedy$useControllerMouseX(int mouseX) {
        return NeoForgeControllerCompat.screenMouseX((Screen) (Object) this, mouseX);
    }

    @ModifyVariable(method = "extractRenderState", at = @At("HEAD"), ordinal = 1, argsOnly = true, remap = false)
    private int raggedy$useControllerMouseY(int mouseY) {
        return NeoForgeControllerCompat.screenMouseY((Screen) (Object) this, mouseY);
    }

    @Inject(method = "extractRenderState", at = @At("TAIL"), remap = false)
    private void raggedy$renderControllerCursor(GuiGraphicsExtractor graphics, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        Screen screen = (Screen) (Object) this;
        if (NeoForgeControllerCompat.shouldRenderCursorInBaseScreen(screen)) {
            NeoForgeControllerCompat.renderCursor(screen, graphics);
        }
    }
}
