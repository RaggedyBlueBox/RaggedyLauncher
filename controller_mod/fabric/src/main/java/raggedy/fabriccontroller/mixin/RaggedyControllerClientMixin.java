package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import net.minecraft.class_310;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(class_310.class)
public abstract class RaggedyControllerClientMixin {
    @Inject(method = "method_1574", at = @At("TAIL"))
    private void raggedy$tickControllerCompat(CallbackInfo ci) {
        RaggedyControllerCompat.tick((class_310)(Object)this);
    }
}
