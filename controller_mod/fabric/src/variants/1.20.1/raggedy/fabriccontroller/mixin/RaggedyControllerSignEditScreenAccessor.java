package raggedy.fabriccontroller.mixin;

import net.minecraft.class_3728;
import net.minecraft.class_7743;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;
import org.spongepowered.asm.mixin.gen.Invoker;

@Mixin(class_7743.class)
public interface RaggedyControllerSignEditScreenAccessor {
    @Accessor("field_40425")
    String[] raggedy$lines();

    @Accessor("field_40428")
    int raggedy$currentLine();

    @Accessor("field_40429")
    class_3728 raggedy$selectionManager();

    @Invoker("method_49913")
    void raggedy$setCurrentLine(String text);

    @Invoker("method_45658")
    boolean raggedy$acceptsLine(String text);
}
