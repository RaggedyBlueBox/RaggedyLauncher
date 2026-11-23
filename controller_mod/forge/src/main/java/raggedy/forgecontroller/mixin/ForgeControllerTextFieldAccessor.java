package raggedy.forgecontroller.mixin;

import net.minecraft.client.gui.components.EditBox;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(EditBox.class)
public interface ForgeControllerTextFieldAccessor {
    @Accessor("f_94094_")
    int raggedy$maxLength();

    @Accessor("f_94101_")
    int raggedy$cursor();

    @Accessor("f_94102_")
    int raggedy$selectionStart();
}
