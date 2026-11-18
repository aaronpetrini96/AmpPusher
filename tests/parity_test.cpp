#include <RTNeural/RTNeural.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using PedalModel = RTNeural::ModelT<float, 3, 1,
    RTNeural::GRULayerT<float, 3, 32>,
    RTNeural::DenseT<float, 32, 1>>;

nlohmann::json loadJson (const std::string& relativePath)
{
    std::ifstream stream (std::string (REPO_ROOT) + "/" + relativePath);
    nlohmann::json json;
    stream >> json;
    return json;
}

void loadModel (PedalModel& model, const nlohmann::json& weights)
{
    RTNeural::torch_helpers::loadGRU<float> (weights, "gru.", model.get<0>());
    RTNeural::torch_helpers::loadDense<float> (weights, "output.", model.get<1>());
}

int main(int argc, char* argv[])
{
    const std::string pedal = argc > 1 ? argv[1] : "ts808";

    PedalModel model;
    loadModel(model, loadJson("models/" + pedal + ".json"));
    model.reset();

    const auto parity = loadJson("tests/data/" + pedal + "_parity.json");
    const auto inputs = parity["inputs"].get<std::vector<std::vector<float>>>();
    const auto expected = parity["expected"].get<std::vector<float>>();

    float maxError = 0.f;
    for (size_t n {0}; n < inputs.size(); ++n)
    {
        maxError = std::max(maxError, std::abs(model.forward(inputs[n].data()) - expected[n]));
    }

    std::cout << pedal << " max error: " << maxError << (maxError < 1e-4f ? " PASS" : " FAIL") << std::endl;
    return maxError < 1e-4f ? 0 : 1;

}