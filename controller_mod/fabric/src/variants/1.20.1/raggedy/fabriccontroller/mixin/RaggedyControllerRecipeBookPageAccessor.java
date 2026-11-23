package raggedy.fabriccontroller.mixin;

import java.util.List;
import net.minecraft.class_361;
import net.minecraft.class_508;
import net.minecraft.class_513;
import net.minecraft.class_514;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(class_513.class)
public interface RaggedyControllerRecipeBookPageAccessor {
    @Accessor("field_3131")
    List<class_514> raggedy$buttons();

    @Accessor("field_3128")
    class_361 raggedy$backButton();

    @Accessor("field_3130")
    class_361 raggedy$forwardButton();

    @Accessor("field_3132")
    class_508 raggedy$overlay();
}
