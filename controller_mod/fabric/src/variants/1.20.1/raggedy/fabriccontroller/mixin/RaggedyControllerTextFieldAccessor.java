package raggedy.fabriccontroller.mixin;

import net.minecraft.class_342;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(class_342.class)
public interface RaggedyControllerTextFieldAccessor {
    @Accessor("field_2108")
    int raggedy$maxLength();

    @Accessor("field_2102")
    int raggedy$cursor();

    @Accessor("field_2101")
    int raggedy$selectionStart();
}
