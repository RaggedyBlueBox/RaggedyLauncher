package raggedy.fabriccontroller.mixin;

import net.minecraft.client.gui.screens.inventory.CreativeModeInventoryScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Invoker;

@Mixin(CreativeModeInventoryScreen.class)
public interface RaggedyControllerCreativeInventoryAccessor {
    @Invoker("refreshSearchResults")
    void raggedy$refreshSearchResults();
}
