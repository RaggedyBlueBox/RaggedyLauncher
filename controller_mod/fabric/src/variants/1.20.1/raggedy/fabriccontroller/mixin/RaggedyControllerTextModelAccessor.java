package raggedy.fabriccontroller.mixin;

import net.minecraft.class_7530;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(class_7530.class)
public interface RaggedyControllerTextModelAccessor {
    @Accessor("field_39516")
    int raggedy$cursor();

    @Accessor("field_39517")
    int raggedy$selectionStart();
}
