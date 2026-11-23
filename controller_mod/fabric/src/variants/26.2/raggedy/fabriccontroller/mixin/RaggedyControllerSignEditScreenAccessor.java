package raggedy.fabriccontroller.mixin;

import net.minecraft.client.gui.font.TextFieldHelper;
import net.minecraft.client.gui.screens.inventory.AbstractSignEditScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;
import org.spongepowered.asm.mixin.gen.Invoker;

@Mixin(AbstractSignEditScreen.class)
public interface RaggedyControllerSignEditScreenAccessor {
    @Accessor("messages")
    String[] raggedy$lines();

    @Accessor("line")
    int raggedy$currentLine();

    @Accessor("signField")
    TextFieldHelper raggedy$selectionManager();

    @Invoker("setMessage")
    void raggedy$setCurrentLine(String text);

    @Invoker("lambda$init$2")
    boolean raggedy$acceptsLine(String text);
}
