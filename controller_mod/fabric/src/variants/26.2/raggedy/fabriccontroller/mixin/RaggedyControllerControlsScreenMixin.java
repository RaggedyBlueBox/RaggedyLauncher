package raggedy.fabriccontroller.mixin;

import raggedy.fabriccontroller.RaggedyControllerCompat;
import raggedy.fabriccontroller.RaggedyControllerSettingsScreen;
import raggedy.fabriccontroller.FabricClientApi;
import java.util.Collections;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.components.AbstractWidget;
import net.minecraft.client.gui.components.OptionsList;
import net.minecraft.client.gui.components.Button;
import net.minecraft.client.gui.screens.Screen;
import net.minecraft.client.gui.screens.options.controls.ControlsScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(ControlsScreen.class)
public abstract class RaggedyControllerControlsScreenMixin {
    @Inject(method = "addOptions", at = @At("TAIL"))
    private void raggedy$addControllerSettingsButton(CallbackInfo ci) {
        Screen screen = (Screen) (Object) this;
        Button button = RaggedyControllerCompat.createButton(0, 0, 150, 20, "Raggedy Controller...", ignored ->
            FabricClientApi.setScreen(Minecraft.getInstance(), new RaggedyControllerSettingsScreen(screen)));
        OptionsList list = ((RaggedyControllerOptionsSubScreenAccessor) this).raggedy$optionsList();
        list.addSmall(Collections.<AbstractWidget>singletonList(button));
    }
}
