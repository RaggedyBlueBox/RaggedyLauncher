package raggedy.fabriccontroller.mixin;

import net.minecraft.client.gui.components.MultilineTextField;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(MultilineTextField.class)
public interface RaggedyControllerTextModelAccessor {
    @Accessor("cursor")
    int raggedy$cursor();

    @Accessor("selectCursor")
    int raggedy$selectionStart();
}
