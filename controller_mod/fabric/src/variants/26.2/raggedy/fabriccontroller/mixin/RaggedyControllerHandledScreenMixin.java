package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import net.minecraft.client.gui.GuiGraphicsExtractor;
import net.minecraft.client.gui.screens.Screen;
import net.minecraft.client.gui.screens.inventory.AbstractContainerScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.ModifyVariable;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(AbstractContainerScreen.class)
public abstract class RaggedyControllerHandledScreenMixin {
    @Inject(method = "extractRenderState", at = @At("HEAD"))
    private void raggedy$updateControllerCursor(GuiGraphicsExtractor context, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        RaggedyControllerCompat.updateScreenCursorBeforeRender((Screen)(Object)this, mouseX, mouseY);
    }

    @ModifyVariable(method = "extractRenderState", at = @At("HEAD"), ordinal = 0, argsOnly = true)
    private int raggedy$useControllerMouseX(int mouseX) {
        return RaggedyControllerCompat.screenMouseX((Screen)(Object)this, mouseX);
    }

    @ModifyVariable(method = "extractRenderState", at = @At("HEAD"), ordinal = 1, argsOnly = true)
    private int raggedy$useControllerMouseY(int mouseY) {
        return RaggedyControllerCompat.screenMouseY((Screen)(Object)this, mouseY);
    }

    @Inject(method = "extractRenderState", at = @At("TAIL"))
    private void raggedy$renderControllerCursor(GuiGraphicsExtractor context, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        RaggedyControllerCompat.renderCursor((Screen)(Object)this, context);
    }
}
