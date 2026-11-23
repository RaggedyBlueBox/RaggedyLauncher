package raggedy.neoforgecontroller.mixin;

import java.util.List;
import net.minecraft.client.gui.components.StateSwitchingButton;
import net.minecraft.client.gui.screens.recipebook.OverlayRecipeComponent;
import net.minecraft.client.gui.screens.recipebook.RecipeBookPage;
import net.minecraft.client.gui.screens.recipebook.RecipeButton;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(value = RecipeBookPage.class, remap = false)
public interface NeoForgeControllerRecipeBookPageAccessor {
    @Accessor("buttons")
    List<RecipeButton> raggedy$buttons();

    @Accessor("forwardButton")
    StateSwitchingButton raggedy$forwardButton();

    @Accessor("backButton")
    StateSwitchingButton raggedy$backButton();

    @Accessor("overlay")
    OverlayRecipeComponent raggedy$overlay();
}
