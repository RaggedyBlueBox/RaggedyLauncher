package raggedy.neoforgecontroller.mixin;

import raggedy.neoforgecontroller.NeoForgeControllerCompat;
import net.minecraft.client.DeltaTracker;
import net.minecraft.client.gui.Gui;
import net.minecraft.client.gui.GuiGraphicsExtractor;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(value = Gui.class, remap = false)
public abstract class NeoForgeControllerHudMixin {
    @Inject(method = "extractRenderState", at = @At("TAIL"), remap = false)
    private void raggedy$renderControllerGuide(GuiGraphicsExtractor context, DeltaTracker tickCounter, CallbackInfo ci) {
        NeoForgeControllerCompat.renderGameplayGuide(context);
    }
}
