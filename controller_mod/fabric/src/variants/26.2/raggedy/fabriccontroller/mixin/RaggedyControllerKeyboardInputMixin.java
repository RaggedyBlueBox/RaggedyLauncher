package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import net.minecraft.world.entity.player.Input;
import net.minecraft.world.phys.Vec2;
import net.minecraft.client.player.KeyboardInput;
import net.minecraft.client.player.ClientInput;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(KeyboardInput.class)
public abstract class RaggedyControllerKeyboardInputMixin extends ClientInput {
    @Inject(method = "tick", at = @At("TAIL"))
    private void raggedy$applyAnalogMovement(CallbackInfo ci) {
        float[] movement = RaggedyControllerCompat.analogMovement();
        if (movement == null || (movement[0] == 0.0f && movement[1] == 0.0f)) {
            return;
        }

        moveVector = new Vec2(movement[0], movement[1]);
        keyPresses = new Input(
            movement[1] > 0.0f,
            movement[1] < 0.0f,
            movement[0] > 0.0f,
            movement[0] < 0.0f,
            keyPresses.jump(),
            keyPresses.shift(),
            keyPresses.sprint());
    }
}
