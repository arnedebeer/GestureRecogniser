from PIL import ImageFont
import visualkeras

from model_constructor import ModelConstructor, ModelName

if __name__ == "__main__":

    model = ModelConstructor.get_model(ModelName.BEERNET_LITE, (20, 5, 3), 10, True)
    font = ImageFont.truetype("Roboto-Regular.ttf", 24)
    
    visualkeras.layered_view(
        model,
        to_file="model-visualisation.png",
        background_fill=(0, 0, 0, 1),
        legend=True, 
        font=font,
        font_color="black", 
        spacing=25,
        max_xy=1000,
        max_z=125,
        scale_xy=32).show()
