package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import net.minecraft.client.gui.GuiGraphicsExtractor;
import net.minecraft.client.gui.Hud;
import net.minecraft.client.DeltaTracker;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(Hud.class)
public abstract class RaggedyControllerHudMixin {
    @Inject(method = "extractRenderState", at = @At("TAIL"))
    private void raggedy$renderControllerGuide(GuiGraphicsExtractor context, DeltaTracker tickCounter, CallbackInfo ci) {
        RaggedyControllerCompat.renderGameplayGuide(context);
    }
}
