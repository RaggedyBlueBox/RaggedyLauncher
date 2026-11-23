package raggedy.fabriccontroller.mixin;

import net.minecraft.class_353;
import net.minecraft.class_4667;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(class_4667.class)
public interface RaggedyControllerOptionsSubScreenAccessor {
    @Accessor("field_51824")
    class_353 raggedy$optionsList();
}
