package raggedy.forgecontroller.mixin;

import net.minecraft.client.gui.font.TextFieldHelper;
import net.minecraft.client.gui.screens.inventory.BookEditScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(BookEditScreen.class)
public interface ForgeControllerBookAccessor {
    @Accessor("f_98067_")
    boolean raggedy$signing();

    @Accessor("f_98072_")
    TextFieldHelper raggedy$page();

    @Accessor("f_98073_")
    TextFieldHelper raggedy$title();
}
