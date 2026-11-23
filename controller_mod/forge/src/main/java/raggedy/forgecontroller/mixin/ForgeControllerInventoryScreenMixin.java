package raggedy.forgecontroller.mixin;

import raggedy.forgecontroller.ForgeControllerCompat;
import net.minecraft.client.gui.GuiGraphics;
import net.minecraft.client.gui.screens.inventory.InventoryScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.ModifyVariable;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(value = InventoryScreen.class, remap = false)
public abstract class ForgeControllerInventoryScreenMixin {
    @Inject(method = "m_88315_", at = @At("HEAD"), remap = false)
    private void raggedy$updateControllerCursor(GuiGraphics graphics, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        ForgeControllerCompat.updateScreenCursorBeforeRender((InventoryScreen) (Object) this, mouseX, mouseY);
    }

    @ModifyVariable(method = "m_88315_", at = @At("HEAD"), ordinal = 0, argsOnly = true, remap = false)
    private int raggedy$useControllerMouseX(int mouseX) {
        return ForgeControllerCompat.screenMouseX(mouseX);
    }

    @ModifyVariable(method = "m_88315_", at = @At("HEAD"), ordinal = 1, argsOnly = true, remap = false)
    private int raggedy$useControllerMouseY(int mouseY) {
        return ForgeControllerCompat.screenMouseY(mouseY);
    }

    @Inject(method = "m_88315_", at = @At("TAIL"), remap = false)
    private void raggedy$renderControllerCursor(GuiGraphics graphics, int mouseX, int mouseY, float delta, CallbackInfo ci) {
        ForgeControllerCompat.renderCursor((InventoryScreen) (Object) this, graphics);
    }
}
