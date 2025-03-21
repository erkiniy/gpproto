/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 4.0.0
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.gp.libclient;

public class gp_data {
  private transient long swigCPtr;
  protected transient boolean swigCMemOwn;

  protected gp_data(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(gp_data obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  @SuppressWarnings("deprecation")
  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        libclientJNI.delete_gp_data(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public void setLength(long value) {
    libclientJNI.gp_data_length_set(swigCPtr, this, value);
  }

  public long getLength() {
    return libclientJNI.gp_data_length_get(swigCPtr, this);
  }

  public void setValue(SWIGTYPE_p_unsigned_char value) {
    libclientJNI.gp_data_value_set(swigCPtr, this, SWIGTYPE_p_unsigned_char.getCPtr(value));
  }

  public SWIGTYPE_p_unsigned_char getValue() {
    long cPtr = libclientJNI.gp_data_value_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_unsigned_char(cPtr, false);
  }

  public gp_data() {
    this(libclientJNI.new_gp_data(), true);
  }

}
