package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import net.minecraft.class_329;
import net.minecraft.class_332;
import net.minecraft.class_9779;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(class_329.class)
public abstract class RaggedyControllerHudMixin {
    @Inject(method = "method_1753", at = @At("TAIL"))
    private void raggedy$renderControllerGuide(class_332 context, class_9779 tickCounter, CallbackInfo ci) {
        RaggedyControllerCompat.renderGameplayGuide(context);
    }
}
