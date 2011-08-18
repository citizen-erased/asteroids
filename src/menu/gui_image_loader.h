#ifndef AST_GUI_IMAGE_LOADER_H
#define AST_GUI_IMAGE_LOADER_H

#include <guichan/imageloader.hpp>
#include <guichan/platform.hpp>

class GuiImageLoader : public gcn::ImageLoader
{
public:
    gcn::Image* load(const std::string& filename, bool convertToDisplayFormat = true);
};

#endif /* AST_GUI_IMAGE_LOADER_H */

