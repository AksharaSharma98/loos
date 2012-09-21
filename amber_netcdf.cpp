// (c) 2012 Tod D. Romo, Grossfield Lab, URMC

#include <amber_netcdf.hpp>
#include <AtomicGroup.hpp>

namespace loos {


  bool isFileNetCDF(const std::string& fname) {
    std::ifstream ifs(fname.c_str());

    char buf[4];
    ifs.read(buf, 4);
    return (buf[0] == 'C' && buf[1] == 'D' && buf[2] == 'F' && (buf[3] = 0x01 || buf[3] == 0x02));
  }




  void AmberNetcdf::init(const char* name, const uint natoms) {
    int retval;

    retval = nc_open(name, NC_NOWRITE, &_ncid);
    if (retval)
      throw(AmberNetcdfOpenError());

    readGlobalAttributes();

    // Verify # of atoms match...
    int atom_id;
    retval = nc_inq_dimid(_ncid, "atom", &atom_id);
    if (retval)
      throw(AmberNetcdfError("Error reading atom id", retval));
    retval = nc_inq_dimlen(_ncid, atom_id, &_natoms);
    if (retval)
      throw(AmberNetcdfError("Error reading atom length", retval));
    if (_natoms != natoms)
      throw(LOOSError("AmberNetcdf has different number of atoms than expected"));


    // Get nframes
    int frame_id;
    retval = nc_inq_dimid(_ncid, "frame", &frame_id);
    if (retval)
      throw(AmberNetcdfError("Error reading frame information", retval));
    retval = nc_inq_dimlen(_ncid, frame_id, &_nframes);

    // Check for periodic cells...
    retval = nc_inq_varid(_ncid, "cell_lengths", &_cell_lengths_id);
    _periodic = !retval;

    // Any additional validation should go here...

    // Get coord-id for later use...
    retval = nc_inq_varid(_ncid, "coordinates", &_coord_id);
    if (retval)
      throw(AmberNetcdfError("Error getting id for coordinates", retval));

    // Now cache the first frame...
    readRawFrame(0);
    cached_first = true;
    
  }


  void AmberNetcdf::readRawFrame(const uint frameno) {
    size_t start[3] = { 0, 0, 0 };
    size_t count[3] = {1, 1, 3};


    // Read coordinates first...
    start[0] = frameno;
    count[1] = _natoms;


    int retval = VarTypeDecider<GCoord::element_type>::read(_ncid, _coord_id, start, count, _coord_data);
    if (retval)
      throw(AmberNetcdfError("Error while reading Amber netcdf frame", retval));

    // Now get box if present...
    if (_periodic) {
      start[1] = 0;
      count[1] = 3;

      retval = VarTypeDecider<GCoord::element_type>::read(_ncid, _cell_lengths_id, start, count, _box_data);
      if (retval)
        throw(AmberNetcdfError("Error while reading Amber netcdf periodic box", retval));
      
    }
    
  }


  void AmberNetcdf::seekNextFrameImpl() {
    ++_current_frame;
  }

  void AmberNetcdf::seekFrameImpl(const uint i) {
    _current_frame = i;
  }

  bool AmberNetcdf::parseFrame() {
    if (_current_frame >= _nframes)
      return(false);

    readRawFrame(_current_frame);
    return(true);
  }

  void AmberNetcdf::updateGroupCoords(AtomicGroup& g) {
    if (g.size() != _natoms)
      throw(AmberNetcdfError("Cannot use AmberNetcdf::updateGroupCoords() when passed group has different number of atoms"));

    uint j=0;
    for (uint i=0; i<_natoms; ++i, j += 3)
      g[i]->coords(GCoord(_coord_data[j], _coord_data[j+1], _coord_data[j+2]));

    if (_periodic)
      g.periodicBox(GCoord(_box_data[0], _box_data[1], _box_data[2]));
  }


  void AmberNetcdf::readGlobalAttributes() {

    _title = readGlobalAttribute("title");
    _application = readGlobalAttribute("application");
    _program = readGlobalAttribute("program");
    _programVersion = readGlobalAttribute("programVersion");
    _conventions = readGlobalAttribute("Conventions");
    _conventionVersion = readGlobalAttribute("ConventionVersion");
  }


  // Will return an emptry string if the attribute is not found
  std::string AmberNetcdf::readGlobalAttribute(const std::string& name) {
    size_t len;
    
    int retval = nc_inq_attlen(_ncid, NC_GLOBAL, name.c_str(), &len);
    if (retval)
      return(std::string());

    nc_type type;
    retval = nc_inq_atttype(_ncid, NC_GLOBAL, name.c_str(), &type);
    if (type != NC_CHAR)
      throw(AmberNetcdfTypeError("Only character data is supported for global attributes"));
    

    char* buf = new char[len+1];
    retval = nc_get_att_text(_ncid, NC_GLOBAL, name.c_str(), buf);
    if (retval) {
      delete[] buf;
      throw(AmberNetcdfError("Error reading attribute " + name));
    }

    return(std::string(buf));
  }





};
