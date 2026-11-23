package raggedy.fabriccontroller.mixin;

import net.minecraft.class_7529;
import net.minecraft.class_7530;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(class_7529.class)
public interface RaggedyControllerEditBoxAccessor {
    @Accessor("field_39509")
    class_7530 raggedy$textModel();
}
