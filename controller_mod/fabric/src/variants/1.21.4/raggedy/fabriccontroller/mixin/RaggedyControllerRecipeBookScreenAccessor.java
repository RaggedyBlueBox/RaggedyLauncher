package raggedy.fabriccontroller.mixin;

import net.minecraft.class_10260;
import net.minecraft.class_507;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(class_10260.class)
public interface RaggedyControllerRecipeBookScreenAccessor {
    @Accessor("field_54474")
    class_507 raggedy$recipeBook();
}
