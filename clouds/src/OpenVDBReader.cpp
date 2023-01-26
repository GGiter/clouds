#include "OpenVDBReader.h"
#define IMATH_HALF_NO_LOOKUP_TABLE
#define _USE_MATH_DEFINES
#include <math.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/ChangeBackground.h>
#pragma comment(lib,"openvdb.lib")

bool OpenVDBReader::Read(const std::string& path, std::vector<float>& data, glm::ivec3& dimensions)
{
	openvdb::initialize();
	openvdb::io::File file(path.c_str());

	if (!file.open()) {
		return false;
	}

	openvdb::GridBase::Ptr baseGrid;
	for (openvdb::io::File::NameIterator nameIter = file.beginName();
		nameIter != file.endName(); ++nameIter)
	{
		baseGrid = file.readGrid(nameIter.gridName());
	}
	file.close();

	openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
	openvdb::OPENVDB_VERSION_NAME::math::Coord dim = grid->evalActiveVoxelDim();
	openvdb::OPENVDB_VERSION_NAME::math::Coord min = grid->evalActiveVoxelBoundingBox().min();
	int dx, dy, dz;
	dx = dim.x();
	dz = dim.z();
	dy = dim.y();

	data.resize(dx * dy * dz * 4);

	openvdb::OPENVDB_VERSION_NAME::FloatTree tree = grid->tree();
	openvdb::OPENVDB_VERSION_NAME::math::Coord cursor;
	for (int z = 0; z < dz; z++) {
		cursor.setZ(min.z() + z);
        int zOffset = z * dx * dy * 4;
		for (int y = 0; y < dy; y++) {
			cursor.setY(min.y() + y);
            int yOffset = y * dx * 4;
			for (int x = 0, i = 0; x < dx; x++, i = i + 4) {
				cursor.setX(min.x() + x);
				float value = tree.getValue(cursor);
				data[i + yOffset + zOffset] = value;
				data[i + 1 + yOffset + zOffset] = value;
				data[i + 2 + yOffset + zOffset] = value;
				data[i + 3 + yOffset + zOffset] = 1.0f;
			}
		}
	}
	dimensions.x = dx;
	dimensions.y = dy;
	dimensions.z = dz;

	return true;
}
