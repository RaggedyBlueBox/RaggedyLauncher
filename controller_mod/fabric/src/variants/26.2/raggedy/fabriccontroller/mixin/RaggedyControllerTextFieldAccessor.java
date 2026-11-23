package raggedy.fabriccontroller.mixin;

import net.minecraft.client.gui.components.EditBox;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(EditBox.class)
public interface RaggedyControllerTextFieldAccessor {
    @Accessor("maxLength")
    int raggedy$maxLength();

    @Accessor("cursorPos")
    int raggedy$cursor();

    @Accessor("highlightPos")
    int raggedy$selectionStart();
}
