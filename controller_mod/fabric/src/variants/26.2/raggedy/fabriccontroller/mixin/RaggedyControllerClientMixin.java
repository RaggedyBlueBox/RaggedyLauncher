package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import net.minecraft.client.Minecraft;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(Minecraft.class)
public abstract class RaggedyControllerClientMixin {
    @Inject(method = "tick", at = @At("TAIL"))
    private void raggedy$tickControllerCompat(CallbackInfo ci) {
        RaggedyControllerCompat.tick((Minecraft)(Object)this);
    }
}
