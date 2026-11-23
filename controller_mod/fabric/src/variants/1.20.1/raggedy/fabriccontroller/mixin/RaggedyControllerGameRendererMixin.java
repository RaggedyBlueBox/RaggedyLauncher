package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import net.minecraft.class_310;
import net.minecraft.class_757;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(class_757.class)
public abstract class RaggedyControllerGameRendererMixin {
    @Shadow
    @Final
    private class_310 field_4015;

    @Inject(method = "method_3192", at = @At("HEAD"))
    private void raggedy$renderControllerLook(float tickDelta, long startTime, boolean tick, CallbackInfo ci) {
        RaggedyControllerCompat.renderFrame(this.field_4015);
    }

    @Inject(method = "method_3192", at = @At(value = "INVOKE", target = "Lnet/minecraft/class_374;method_1996(Lnet/minecraft/class_332;)V", shift = At.Shift.AFTER))
    private void raggedy$renderControllerCursor(float tickDelta, long startTime, boolean tick, CallbackInfo ci) {
        RaggedyControllerCompat.renderQueuedCursor();
    }
}
