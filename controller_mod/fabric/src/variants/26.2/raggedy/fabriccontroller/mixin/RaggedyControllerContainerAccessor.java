package raggedy.fabriccontroller.mixin;

import net.minecraft.world.inventory.ContainerInput;
import net.minecraft.world.inventory.Slot;
import net.minecraft.client.gui.screens.inventory.AbstractContainerScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;
import org.spongepowered.asm.mixin.gen.Invoker;

@Mixin(AbstractContainerScreen.class)
public interface RaggedyControllerContainerAccessor {
    @Accessor("leftPos")
    int raggedy$leftPos();

    @Accessor("topPos")
    int raggedy$topPos();

    @Accessor("hoveredSlot")
    Slot raggedy$getSlotUnderMouse();

    @Invoker("slotClicked")
    void raggedy$slotClicked(Slot slot, int slotId, int button, ContainerInput actionType);
}
