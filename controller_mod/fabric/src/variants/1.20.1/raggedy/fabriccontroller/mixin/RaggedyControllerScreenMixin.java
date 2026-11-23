package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import net.minecraft.class_332;
import net.minecraft.class_437;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.ModifyVariable;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(class_437.class)
public abstract class RaggedyControllerScreenMixin {
    @Inject(method = "method_25394", at = @At("HEAD"))
    private void raggedy$updateControllerCursor(class_332 context, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        RaggedyControllerCompat.updateScreenCursorBeforeRender((class_437)(Object)this, mouseX, mouseY);
    }

    @ModifyVariable(method = "method_25394", at = @At("HEAD"), ordinal = 0, argsOnly = true)
    private int raggedy$useControllerMouseX(int mouseX) {
        return RaggedyControllerCompat.screenMouseX((class_437)(Object)this, mouseX);
    }

    @ModifyVariable(method = "method_25394", at = @At("HEAD"), ordinal = 1, argsOnly = true)
    private int raggedy$useControllerMouseY(int mouseY) {
        return RaggedyControllerCompat.screenMouseY((class_437)(Object)this, mouseY);
    }

    @Inject(method = "method_25394", at = @At("TAIL"))
    private void raggedy$renderControllerCursor(class_332 context, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        class_437 screen = (class_437)(Object)this;
        if (RaggedyControllerCompat.shouldRenderCursorInBaseScreen(screen)) {
            RaggedyControllerCompat.renderCursor(screen, context);
        }
    }
}
