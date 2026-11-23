package raggedy.neoforgecontroller.mixin;

import net.minecraft.client.gui.components.MultilineTextField;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(MultilineTextField.class)
public interface NeoForgeControllerTextModelAccessor {
    @Accessor("cursor")
    int raggedy$cursor();

    @Accessor("selectCursor")
    int raggedy$selectionStart();
}
