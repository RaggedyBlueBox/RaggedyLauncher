package raggedy.neoforgecontroller.mixin;

import java.util.List;
import net.minecraft.client.gui.components.EditBox;
import net.minecraft.client.gui.components.StateSwitchingButton;
import net.minecraft.client.gui.screens.recipebook.RecipeBookComponent;
import net.minecraft.client.gui.screens.recipebook.RecipeBookPage;
import net.minecraft.client.gui.screens.recipebook.RecipeBookTabButton;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(value = RecipeBookComponent.class, remap = false)
public interface NeoForgeControllerRecipeBookAccessor {
    @Accessor("searchBox")
    EditBox raggedy$searchBox();

    @Accessor("filterButton")
    StateSwitchingButton raggedy$filterButton();

    @Accessor("tabButtons")
    List<RecipeBookTabButton> raggedy$tabButtons();

    @Accessor("recipeBookPage")
    RecipeBookPage raggedy$recipeBookPage();
}
