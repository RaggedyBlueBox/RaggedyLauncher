package raggedy.neoforgecontroller.mixin;

import net.minecraft.client.gui.font.TextFieldHelper;
import net.minecraft.client.gui.screens.inventory.BookEditScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(BookEditScreen.class)
public interface NeoForgeControllerBookAccessor {
    @Accessor("isSigning")
    boolean raggedy$signing();

    @Accessor("pageEdit")
    TextFieldHelper raggedy$page();

    @Accessor("titleEdit")
    TextFieldHelper raggedy$title();
}
