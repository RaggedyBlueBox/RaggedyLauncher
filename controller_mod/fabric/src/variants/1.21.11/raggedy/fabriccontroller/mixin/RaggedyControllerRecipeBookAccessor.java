package raggedy.fabriccontroller.mixin;

import java.util.List;
import net.minecraft.class_342;
import net.minecraft.class_507;
import net.minecraft.class_512;
import net.minecraft.class_513;
import net.minecraft.class_5676;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(class_507.class)
public interface RaggedyControllerRecipeBookAccessor {
    @Accessor("field_3089")
    class_342 raggedy$searchBox();

    @Accessor("field_3088")
    class_5676<?> raggedy$filterButton();

    @Accessor("field_3094")
    List<class_512> raggedy$tabButtons();

    @Accessor("field_3086")
    class_513 raggedy$recipeBookPage();
}
