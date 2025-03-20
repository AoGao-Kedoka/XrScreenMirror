#include "CustomPass.h"
#include "ICapture.h"
#include "XRLib.h"

#if defined(_WIN32) || defined(_WIN64)
#include "WindowsCapture.h"
#elif defined(__linux__)
#include "LinuxCapture.h"
#endif

void UpdateImageBuffer(XRLib::Graphics::VkCore& core, XRLib::Graphics::Image& img,
                       XRLib::Graphics::Buffer& deviceBuffer);

int main() {
    XRLib::XRLib xrLib;

    xrLib.SetVersionNumber(1, 0, 0)
        .SetApplicationName("XrScreenMirror")
        .SetCustomOpenXRRuntime("~/Tools/MetaXRSimulator/meta_openxr_simulator.json");
    

    XRLib::Transform planeTransform;

    // initialize capture
    std::unique_ptr<ICapture> capture;
#if defined(_WIN32) || defined(_WIN64)
    capture = std::make_unique<WindowsCapture>();
#elif defined(__linux__)
    capture = std::make_unique<LinuxCapture>();
#endif

    // initialize custom pass
    auto customRenderBehavior = std::make_unique<CustomPass>(xrLib.GetVkCore(), xrLib.SceneBackend(),
                                                             &xrLib.RenderBackend().RenderPasses, true);
    auto rawCustomRenderBehaviorPtr = customRenderBehavior.get();

    // define transformations and load meshes
    planeTransform.Scale(glm::vec3(0.4, 0.2, 0.05)).Rotate(glm::vec3(1, 0, 0), 90).Translate(glm::vec3(0, -100, -10));

    XRLib::Entity* plane{nullptr};
    XRLib::Entity* leftHand{nullptr};
    XRLib::Entity* rightHand{nullptr};

    xrLib.SceneBackend()
        .LoadMeshAsyncWithBinding({"./resources/plane.obj", planeTransform}, plane)
        .LoadMeshAsyncWithBinding({"./resources/leftHand.glb"}, leftHand)
        .AttachEntityToLeftControllerPose(leftHand)
        .LoadMeshAsyncWithBinding({"./resources/rightHand.glb"}, rightHand)
        .AttachEntityToRightcontrollerPose(rightHand)
        .WaitForAllMeshesToLoad();

    std::cout << xrLib.SceneBackend();

    auto planeMesh = static_cast<XRLib::Mesh*>(plane);
    auto data = capture->CaptureScreen();

    planeMesh->Diffuse = {data, capture->Width, capture->Height, 4};

    // initialize lib
    xrLib.Init(true, std::move(customRenderBehavior));

    if (!xrLib.GetXrCore().IsXRValid()) {
        std::cout << "XR is not valid" << std::endl;
        return -1;
    }

    // render loop
    while (!xrLib.ShouldStop()) {
        data = capture->CaptureScreen();
        auto size = capture->Width * capture->Height * 4;
        std::unique_ptr<XRLib::Graphics::Buffer> imageBuffer = std::make_unique<XRLib::Graphics::Buffer>(
            xrLib.GetVkCore(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, static_cast<void*>(data.data()), false);
        UpdateImageBuffer(xrLib.GetVkCore(), *rawCustomRenderBehaviorPtr->textures[0], *imageBuffer);

        xrLib.Run();
    }

    return 0;
}

void UpdateImageBuffer(XRLib::Graphics::VkCore& core, XRLib::Graphics::Image& img,
                       XRLib::Graphics::Buffer& hostBuffer) {
    auto format = img.GetFormat();

    // transition from image layout shader read only to transfer destination (not implemented in XRLib, hence manual transition)
    auto commandBuffer = XRLib::Graphics::CommandBuffer::BeginSingleTimeCommands(core);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = img.GetImage();

    // Define the affected aspect of the image
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    // Synchronization: Prevent reads before the transition
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    // Pipeline stage synchronization
    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    // Submit the barrier
    vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);

    XRLib::Graphics::CommandBuffer::EndSingleTimeCommands(commandBuffer);

    img.CopyBufferToImage(hostBuffer.GetBuffer(), img.Width(), img.Height());

    img.TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
