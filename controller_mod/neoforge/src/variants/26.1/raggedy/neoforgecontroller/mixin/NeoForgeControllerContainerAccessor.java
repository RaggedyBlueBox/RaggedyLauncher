package raggedy.neoforgecontroller.mixin;

import net.minecraft.client.gui.screens.inventory.AbstractContainerScreen;
import net.minecraft.world.inventory.ContainerInput;
import net.minecraft.world.inventory.Slot;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.gen.Accessor;
import org.spongepowered.asm.mixin.gen.Invoker;

@Mixin(value = AbstractContainerScreen.class, remap = false)
public interface NeoForgeControllerContainerAccessor {
    @Accessor("leftPos")
    int raggedy$leftPos();

    @Accessor("topPos")
    int raggedy$topPos();

    @Accessor("hoveredSlot")
    Slot raggedy$getSlotUnderMouse();

    @Invoker("slotClicked")
    void raggedy$slotClicked(Slot slot, int slotId, int button, ContainerInput actionType);
}
