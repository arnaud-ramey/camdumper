/*!
  \file        camera_opencv_sample.cpp
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2013/9/30

________________________________________________________________________________

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
________________________________________________________________________________

A camera skeleton, based on:
http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html
 */
// C
#include <sys/time.h>
#include <stdio.h>
// C++
#include <iomanip>      // std::setfill, std::setw
#include <opencv2/highgui/highgui.hpp>

class Timer {
public:
  typedef double Time;
  static const Time NOTIME = -1;
  Timer() { reset(); }
  virtual inline void reset() {
    gettimeofday(&start, NULL);
  }
  //! get the time since ctor or last reset (milliseconds)
  virtual inline Time getTimeSeconds() const {
    struct timeval end;
    gettimeofday(&end, NULL);
    return (Time) (// seconds
                   (end.tv_sec - start.tv_sec)
                   +
                   // useconds
                   (end.tv_usec - start.tv_usec)
                   / 1E6);
  }
private:
  struct timeval start;
}; // end class Timer

////////////////////////////////////////////////////////////////////////////////

/*!
 \return std::string
    the current time, in format: yy-mm-dd_hh-mn-sc-msec
    It only contains characters friendly with a filename.
 \see http://www.cplusplus.com/reference/clibrary/ctime/strftime/
*/
inline std::string timestamp() {
  /* Obtain the time of day, and convert it to a tm struct. */
  struct timeval tv;
  gettimeofday (&tv, NULL);
  struct tm* ptm = localtime (&tv.tv_sec);
  /* Format the date and time, down to a single second. */
  char time_string[40];
  strftime (time_string, sizeof (time_string), "%Y-%m-%d_%H-%M-%S", ptm);
  /* Compute milliseconds from microseconds. */
  int milliseconds = (int) tv.tv_usec / 1000;
  std::ostringstream ans;
  // add the milliseconds with eventual leading zeros
  ans << time_string << "-"
      << std::setw( 3 ) << std::setfill( '0' ) << milliseconds;
  return ans.str();
} // end timestamp()

////////////////////////////////////////////////////////////////////////////////

int main(int, char**) {
  bool display = false, use_buffer = true;
  cv::VideoCapture cap(0); // open the default camera
  int w = 640, h = 480;
  //int w = 1600, h = 1200;
  printf("w:%i, h:%i\n", w, h);
  assert(cap.isOpened());  // check if we succeeded
  cap.set(CV_CAP_PROP_FRAME_WIDTH, w);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, h);

  unsigned int BUFFER_MAX_SIZE = 100, frame_counter = 0;
  std::vector<cv::Mat> frames_buffer;
  std::vector<std::string> frame_files;
  if (!use_buffer)
    printf("Not using buffers, encoding on the fly\n");

  Timer timer;
  cv::namedWindow("frame",1);
  while(frame_counter < BUFFER_MAX_SIZE) {
    cv::Mat frame;
    cap >> frame; // get a new frame from camera
    if (use_buffer && frames_buffer.empty()) {
      printf("Creating buffers...\n");
      frame_files.resize(BUFFER_MAX_SIZE);
      frames_buffer.resize(BUFFER_MAX_SIZE);
      for (unsigned int i = 0; i < BUFFER_MAX_SIZE; ++i)
        frame.copyTo(frames_buffer[i]);
      printf("Creating buffer done.\n");
    }

    std::ostringstream filename;
    filename << "/tmp/" << timestamp() << ".png";
    if (use_buffer) {
      frame.copyTo(frames_buffer[frame_counter]);
      frame_files[frame_counter] = filename.str();
    }
    else {
      cv::imwrite(filename.str(), frame);
    }

    ++frame_counter;
    if (frame_counter % 10 == 0) {
      printf("Moving average fps:%g\n", 10. / timer.getTimeSeconds());
      timer.reset();
    }

    // display
    if (!display)
      continue;
    cv::imshow("frame", frame);
    if(cv::waitKey(5) >= 0) break;
  }
  cap.release();
  // save buffer
  for (unsigned int i = 0; i < frame_counter; ++i) {
    if (i % 10 == 0)
      printf("Written %i of %i files\n", i, frame_counter);
    cv::putText(frames_buffer[i], frame_files[i], cv::Point(10, 20),
                CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0, 255, 0));
    cv::imwrite(frame_files[i], frames_buffer[i]);
  } // end for i

  // the camera will be deinitialized automatically in VideoCapture destructor
  return 0;
}
