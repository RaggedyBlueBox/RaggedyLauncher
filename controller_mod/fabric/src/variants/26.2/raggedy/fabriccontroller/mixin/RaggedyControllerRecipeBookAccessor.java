package raggedy.fabriccontroller.mixin;

import java.util.List;
import net.minecraft.client.gui.components.EditBox;
import net.minecraft.client.gui.screens.recipebook.RecipeBookComponent;
import net.minecraft.client.gui.screens.recipebook.RecipeBookTabButton;
import net.minecraft.client.gui.screens.recipebook.RecipeBookPage;
import net.minecraft.client.gui.components.CycleButton;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;

@Mixin(RecipeBookComponent.class)
public interface RaggedyControllerRecipeBookAccessor {
    @Accessor("searchBox")
    EditBox raggedy$searchBox();

    @Accessor("filterButton")
    CycleButton<?> raggedy$filterButton();

    @Accessor("tabButtons")
    List<RecipeBookTabButton> raggedy$tabButtons();

    @Accessor("recipeBookPage")
    RecipeBookPage raggedy$recipeBookPage();
}
