package raggedy.fabriccontroller.mixin;

import net.minecraft.client.gui.components.OptionsList;
import net.minecraft.client.gui.screens.options.OptionsSubScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(OptionsSubScreen.class)
public interface RaggedyControllerOptionsSubScreenAccessor {
    @Accessor("list")
    OptionsList raggedy$optionsList();
}
