package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import net.minecraft.class_743;
import net.minecraft.class_744;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(class_743.class)
public abstract class RaggedyControllerKeyboardInputMixin extends class_744 {
    @Inject(method = "method_3129", at = @At("TAIL"))
    private void raggedy$applyAnalogMovement(boolean slowDown, float slowDownFactor, CallbackInfo ci) {
        float[] movement = RaggedyControllerCompat.analogMovement();
        if (movement == null || (movement[0] == 0.0f && movement[1] == 0.0f)) {
            return;
        }

        field_3907 = movement[0] * (slowDown ? slowDownFactor : 1.0f);
        field_3905 = movement[1] * (slowDown ? slowDownFactor : 1.0f);
        field_3910 = movement[1] > 0.0f;
        field_3909 = movement[1] < 0.0f;
        field_3908 = movement[0] > 0.0f;
        field_3906 = movement[0] < 0.0f;
    }
}